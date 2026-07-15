#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Store WiFi station credentials in NVS. */
esp_err_t ota_wifi_set_credentials(const char *ssid, const char *password);

/** Bring up WiFi station using stored credentials; blocks until IP or timeout. */
esp_err_t ota_wifi_join(void);

/**
 * Download and apply a firmware image from `url` (http:// or https://),
 * write it to the passive OTA slot, and reboot into it on success.
 * Requires ota_wifi_join() first.
 */
esp_err_t ota_run(const char *url);

/**
 * Confirm the currently running image after a successful boot so the
 * bootloader cancels rollback. Call once from app_main after core
 * subsystems come up.
 */
void ota_mark_boot_valid(void);

#ifdef __cplusplus
}
#endif
