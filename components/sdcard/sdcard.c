#include "sdcard.h"

#include "board_pins.h"
#include "driver/sdmmc_host.h"
#include "esp_check.h"
#include "esp_log.h"

static const char *TAG = "sdcard";

static sdmmc_card_t s_card;
static bool s_ready;

esp_err_t sdcard_init(void)
{
    if (s_ready) {
        return ESP_OK;
    }

    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;

    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.width = 4;
    slot_config.clk = PIN_SD_CLK;
    slot_config.cmd = PIN_SD_CMD;
    slot_config.d0 = PIN_SD_D0;
    slot_config.d1 = PIN_SD_D1;
    slot_config.d2 = PIN_SD_D2;
    slot_config.d3 = PIN_SD_D3;
    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

    ESP_RETURN_ON_ERROR(sdmmc_host_init(), TAG, "host init failed");
    ESP_RETURN_ON_ERROR(sdmmc_host_init_slot(SDMMC_HOST_SLOT_1, &slot_config),
                        TAG, "slot init failed");
    ESP_RETURN_ON_ERROR(sdmmc_card_init(&host, &s_card), TAG, "card init failed");

    sdmmc_card_print_info(stdout, &s_card);
    s_ready = true;
    return ESP_OK;
}

sdmmc_card_t *sdcard_get(void)
{
    return s_ready ? &s_card : NULL;
}
