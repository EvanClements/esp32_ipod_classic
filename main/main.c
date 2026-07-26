#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "track_utils.h"

static const char *TAG = "ipod";

void app_main(void)
{
    char elapsed[16];
    track_utils_format_time(0, elapsed, sizeof(elapsed));

    ESP_LOGI(TAG, "esp32_ipod_classic booting (now playing %s)", elapsed);
}
