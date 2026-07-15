#include "ota.h"

#include <string.h>

#include "esp_check.h"
#include "esp_crt_bundle.h"
#include "esp_event.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_ota_ops.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "nvs.h"
#include "nvs_flash.h"

static const char *TAG = "ota";

#define NVS_NAMESPACE "wifi"
#define WIFI_CONNECT_TIMEOUT_MS 15000
#define WIFI_CONNECTED_BIT BIT0

static EventGroupHandle_t s_wifi_events;
static bool s_wifi_started;

esp_err_t ota_wifi_set_credentials(const char *ssid, const char *password)
{
    nvs_handle_t nvs;
    ESP_RETURN_ON_ERROR(nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs), TAG, "nvs open failed");
    esp_err_t err = nvs_set_str(nvs, "ssid", ssid);
    if (err == ESP_OK) {
        err = nvs_set_str(nvs, "pass", password);
    }
    if (err == ESP_OK) {
        err = nvs_commit(nvs);
    }
    nvs_close(nvs);
    return err;
}

static void wifi_event_handler(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        xEventGroupClearBits(s_wifi_events, WIFI_CONNECTED_BIT);
        esp_wifi_connect();
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)data;
        ESP_LOGI(TAG, "got IP " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_events, WIFI_CONNECTED_BIT);
    }
}

esp_err_t ota_wifi_join(void)
{
    wifi_config_t wifi_config = {0};

    nvs_handle_t nvs;
    ESP_RETURN_ON_ERROR(nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs),
                        TAG, "no stored credentials — use wifi_set first");
    size_t ssid_len = sizeof(wifi_config.sta.ssid);
    size_t pass_len = sizeof(wifi_config.sta.password);
    esp_err_t err = nvs_get_str(nvs, "ssid", (char *)wifi_config.sta.ssid, &ssid_len);
    if (err == ESP_OK) {
        err = nvs_get_str(nvs, "pass", (char *)wifi_config.sta.password, &pass_len);
    }
    nvs_close(nvs);
    ESP_RETURN_ON_ERROR(err, TAG, "reading credentials failed");

    if (!s_wifi_started) {
        s_wifi_events = xEventGroupCreate();
        ESP_RETURN_ON_ERROR(esp_netif_init(), TAG, "netif init failed");
        ESP_RETURN_ON_ERROR(esp_event_loop_create_default(), TAG, "event loop failed");
        esp_netif_create_default_wifi_sta();

        wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_RETURN_ON_ERROR(esp_wifi_init(&init_cfg), TAG, "wifi init failed");
        ESP_RETURN_ON_ERROR(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                       wifi_event_handler, NULL),
                            TAG, "handler register failed");
        ESP_RETURN_ON_ERROR(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                                       wifi_event_handler, NULL),
                            TAG, "handler register failed");
        ESP_RETURN_ON_ERROR(esp_wifi_set_mode(WIFI_MODE_STA), TAG, "set mode failed");
        ESP_RETURN_ON_ERROR(esp_wifi_set_config(WIFI_IF_STA, &wifi_config),
                            TAG, "set config failed");
        ESP_RETURN_ON_ERROR(esp_wifi_start(), TAG, "wifi start failed");
        s_wifi_started = true;
    } else {
        ESP_RETURN_ON_ERROR(esp_wifi_set_config(WIFI_IF_STA, &wifi_config),
                            TAG, "set config failed");
        esp_wifi_connect();
    }

    EventBits_t bits = xEventGroupWaitBits(s_wifi_events, WIFI_CONNECTED_BIT,
                                           pdFALSE, pdTRUE,
                                           pdMS_TO_TICKS(WIFI_CONNECT_TIMEOUT_MS));
    ESP_RETURN_ON_FALSE(bits & WIFI_CONNECTED_BIT, ESP_ERR_TIMEOUT, TAG,
                        "WiFi connect timed out");
    return ESP_OK;
}

esp_err_t ota_run(const char *url)
{
    ESP_LOGI(TAG, "starting OTA from %s", url);

    esp_http_client_config_t http_config = {
        .url = url,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .keep_alive_enable = true,
        .timeout_ms = 10000,
    };
    esp_https_ota_config_t ota_config = {
        .http_config = &http_config,
    };

    esp_err_t err = esp_https_ota(&ota_config);
    ESP_RETURN_ON_ERROR(err, TAG, "OTA failed");

    ESP_LOGI(TAG, "OTA OK, rebooting into new image");
    esp_restart();
    return ESP_OK; /* unreachable */
}

void ota_mark_boot_valid(void)
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t state;
    if (esp_ota_get_state_partition(running, &state) == ESP_OK &&
        state == ESP_OTA_IMG_PENDING_VERIFY) {
        ESP_LOGI(TAG, "first boot of new image OK — cancelling rollback");
        esp_ota_mark_app_valid_cancel_rollback();
    }
}
