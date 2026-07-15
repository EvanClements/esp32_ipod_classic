#pragma once

#include "esp_err.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Mount point for the SD card filesystem when the app owns it. */
#define USB_MSC_MOUNT_POINT "/sdcard"

/**
 * Called when storage ownership changes.
 * app_owns == true  -> filesystem mounted at /sdcard, app may read files
 * app_owns == false -> USB host took the card; all file access must stop
 * Runs in TinyUSB task context — keep it short, no blocking file I/O.
 */
typedef void (*usb_msc_ownership_cb_t)(bool app_owns);

/**
 * Install TinyUSB with an MSC function exposing the raw SD card, then mount
 * the FAT filesystem at /sdcard for the app. When a USB host connects and
 * claims the storage, the filesystem is unmounted automatically and the
 * callback fires with app_owns == false.
 */
esp_err_t usb_msc_init(usb_msc_ownership_cb_t cb);

/** True when the filesystem is mounted for the app (USB host not using it). */
bool usb_msc_app_owns_storage(void);

#ifdef __cplusplus
}
#endif
