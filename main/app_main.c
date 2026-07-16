#include "audio.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "ota.h"
#include "sdcard.h"
#include "usb_msc.h"

static const char *TAG = "main";

void console_start(void);

/* USB host plugged in / unplugged: playback must not touch the card while
 * the host owns it. */
static void on_storage_ownership_change(bool app_owns)
{
    if (!app_owns) {
        audio_stop();
    }
}

void app_main(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    err = sdcard_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "SD card init failed (%s) — insert card and reboot",
                 esp_err_to_name(err));
    } else {
        err = usb_msc_init(on_storage_ownership_change);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "USB MSC init failed (%s)", esp_err_to_name(err));
        }
    }

    /* Boot is functional — confirm image so the bootloader cancels rollback. */
    ota_mark_boot_valid();

    console_start();
}
