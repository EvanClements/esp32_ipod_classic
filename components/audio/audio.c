#include "audio.h"

#include <math.h>
#include <string.h>

#include "audio_element.h"
#include "audio_event_iface.h"
#include "audio_pipeline.h"
#include "board_pins.h"
#include "driver/i2s_std.h"
#include "esp_check.h"
#include "esp_decoder.h"
#include "esp_log.h"
#include "fatfs_stream.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "i2s_stream.h"

static const char *TAG = "audio";

static audio_pipeline_handle_t s_pipeline;
static audio_element_handle_t s_fatfs;
static audio_element_handle_t s_decoder;
static audio_element_handle_t s_i2s;
static audio_event_iface_handle_t s_evt;
static SemaphoreHandle_t s_lock;
static volatile bool s_playing;
static bool s_started_once;

static void audio_event_task(void *arg)
{
    while (true) {
        audio_event_iface_msg_t msg;
        if (audio_event_iface_listen(s_evt, &msg, portMAX_DELAY) != ESP_OK) {
            continue;
        }

        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT &&
            msg.source == (void *)s_decoder &&
            msg.cmd == AEL_MSG_CMD_REPORT_MUSIC_INFO) {
            audio_element_info_t info = {0};
            audio_element_getinfo(s_decoder, &info);
            ESP_LOGI(TAG, "track: %d Hz, %d bit, %d ch",
                     info.sample_rates, info.bits, info.channels);
            i2s_stream_set_clk(s_i2s, info.sample_rates, info.bits, info.channels);
        }

        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT &&
            msg.source == (void *)s_i2s &&
            msg.cmd == AEL_MSG_CMD_REPORT_STATUS) {
            audio_element_status_t status = (audio_element_status_t)(intptr_t)msg.data;
            if (status == AEL_STATUS_STATE_FINISHED || status == AEL_STATUS_STATE_STOPPED) {
                ESP_LOGI(TAG, "playback %s",
                         status == AEL_STATUS_STATE_FINISHED ? "finished" : "stopped");
                s_playing = false;
            }
        }
    }
}

esp_err_t audio_init(void)
{
    if (s_pipeline != NULL) {
        return ESP_OK;
    }

    s_lock = xSemaphoreCreateMutex();
    ESP_RETURN_ON_FALSE(s_lock, ESP_ERR_NO_MEM, TAG, "mutex alloc failed");

    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    s_pipeline = audio_pipeline_init(&pipeline_cfg);
    ESP_RETURN_ON_FALSE(s_pipeline, ESP_FAIL, TAG, "pipeline init failed");

    fatfs_stream_cfg_t fatfs_cfg = FATFS_STREAM_CFG_DEFAULT();
    fatfs_cfg.type = AUDIO_STREAM_READER;
    s_fatfs = fatfs_stream_init(&fatfs_cfg);
    ESP_RETURN_ON_FALSE(s_fatfs, ESP_FAIL, TAG, "fatfs stream init failed");

    /* One auto-detecting decoder element: probes the stream and dispatches
     * to the right codec, so V0.5's extra formats are already covered. */
    audio_decoder_t decoders[] = {
        DEFAULT_ESP_MP3_DECODER_CONFIG(),
        DEFAULT_ESP_FLAC_DECODER_CONFIG(),
        DEFAULT_ESP_WAV_DECODER_CONFIG(),
        DEFAULT_ESP_AAC_DECODER_CONFIG(),
        DEFAULT_ESP_M4A_DECODER_CONFIG(),
        DEFAULT_ESP_OGG_DECODER_CONFIG(),
    };
    esp_decoder_cfg_t dec_cfg = DEFAULT_ESP_DECODER_CONFIG();
    s_decoder = esp_decoder_init(&dec_cfg, decoders,
                                 sizeof(decoders) / sizeof(decoders[0]));
    ESP_RETURN_ON_FALSE(s_decoder, ESP_FAIL, TAG, "decoder init failed");

    /* Pins come from get_i2s_pins() in components/board (custom audio board). */
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg.type = AUDIO_STREAM_WRITER;
    s_i2s = i2s_stream_init(&i2s_cfg);
    ESP_RETURN_ON_FALSE(s_i2s, ESP_FAIL, TAG, "i2s stream init failed");

    audio_pipeline_register(s_pipeline, s_fatfs, "file");
    audio_pipeline_register(s_pipeline, s_decoder, "dec");
    audio_pipeline_register(s_pipeline, s_i2s, "i2s");
    const char *link_tag[3] = {"file", "dec", "i2s"};
    ESP_RETURN_ON_ERROR(audio_pipeline_link(s_pipeline, link_tag, 3),
                        TAG, "pipeline link failed");

    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    s_evt = audio_event_iface_init(&evt_cfg);
    ESP_RETURN_ON_FALSE(s_evt, ESP_FAIL, TAG, "event iface init failed");
    audio_pipeline_set_listener(s_pipeline, s_evt);

    BaseType_t ok = xTaskCreate(audio_event_task, "audio_evt", 4096, NULL, 5, NULL);
    ESP_RETURN_ON_FALSE(ok == pdPASS, ESP_ERR_NO_MEM, TAG, "event task create failed");

    ESP_LOGI(TAG, "pipeline ready: fatfs -> esp_decoder -> i2s");
    return ESP_OK;
}

esp_err_t audio_play(const char *path)
{
    ESP_RETURN_ON_ERROR(audio_init(), TAG, "init failed");
    xSemaphoreTake(s_lock, portMAX_DELAY);

    if (s_started_once) {
        audio_pipeline_stop(s_pipeline);
        audio_pipeline_wait_for_stop(s_pipeline);
        audio_pipeline_terminate(s_pipeline);
    }

    audio_element_set_uri(s_fatfs, path);
    audio_pipeline_reset_ringbuffer(s_pipeline);
    audio_pipeline_reset_elements(s_pipeline);

    esp_err_t err = audio_pipeline_run(s_pipeline);
    if (err == ESP_OK) {
        s_started_once = true;
        s_playing = true;
        ESP_LOGI(TAG, "playing %s", path);
    }

    xSemaphoreGive(s_lock);
    return err;
}

esp_err_t audio_stop(void)
{
    if (s_pipeline == NULL || !s_playing) {
        return ESP_OK;
    }
    xSemaphoreTake(s_lock, portMAX_DELAY);
    audio_pipeline_stop(s_pipeline);
    audio_pipeline_wait_for_stop(s_pipeline);
    audio_pipeline_terminate(s_pipeline);
    s_playing = false;
    xSemaphoreGive(s_lock);
    return ESP_OK;
}

bool audio_is_playing(void)
{
    return s_playing;
}

/* --- Sine smoke test ------------------------------------------------------
 * Uses a raw i2s_std channel on the DAC pins. Only valid before the ADF
 * pipeline claims the I2S port, i.e. before the first audio_play(). */

esp_err_t audio_sine_test(int seconds)
{
    ESP_RETURN_ON_FALSE(s_pipeline == NULL, ESP_ERR_INVALID_STATE, TAG,
                        "sine test only available before playback pipeline starts");

    const int rate = 44100;
    const float freq = 440.0f;

    i2s_chan_handle_t tx = NULL;
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    ESP_RETURN_ON_ERROR(i2s_new_channel(&chan_cfg, &tx, NULL), TAG, "channel alloc failed");

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(rate),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT,
                                                        I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = PIN_I2S_BCK,
            .ws = PIN_I2S_LRCK,
            .dout = PIN_I2S_DOUT,
            .din = I2S_GPIO_UNUSED,
        },
    };
    esp_err_t err = i2s_channel_init_std_mode(tx, &std_cfg);
    if (err == ESP_OK) {
        err = i2s_channel_enable(tx);
    }

    if (err == ESP_OK) {
        enum { FRAMES = 512 };
        static int16_t buf[FRAMES * 2];
        float phase = 0.0f;
        const float step = 2.0f * (float)M_PI * freq / rate;
        int total_frames = rate * seconds;

        for (int written = 0; written < total_frames && err == ESP_OK; written += FRAMES) {
            for (int i = 0; i < FRAMES; i++) {
                int16_t sample = (int16_t)(sinf(phase) * 12000.0f);
                buf[2 * i] = sample;
                buf[2 * i + 1] = sample;
                phase += step;
                if (phase > 2.0f * (float)M_PI) {
                    phase -= 2.0f * (float)M_PI;
                }
            }
            size_t bytes_written = 0;
            err = i2s_channel_write(tx, buf, sizeof(buf), &bytes_written, portMAX_DELAY);
        }
        i2s_channel_disable(tx);
    }

    i2s_del_channel(tx);
    return err;
}
