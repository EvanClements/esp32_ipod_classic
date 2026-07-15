#include "usb_msc.h"

#include "esp_check.h"
#include "esp_log.h"
#include "sdcard.h"
#include "tinyusb.h"
#include "tusb_msc_storage.h"

static const char *TAG = "usb_msc";

static usb_msc_ownership_cb_t s_ownership_cb;
static volatile bool s_app_owns;

/* --- USB descriptors ----------------------------------------------------- */

enum {
    ITF_NUM_MSC = 0,
    ITF_NUM_TOTAL,
};

enum {
    EDPT_MSC_OUT = 0x01,
    EDPT_MSC_IN = 0x81,
};

#define TUSB_DESC_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_MSC_DESC_LEN)

static const tusb_desc_device_t device_descriptor = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = 0x303A, /* Espressif VID */
    .idProduct = 0x4002,
    .bcdDevice = 0x0100,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 0x01,
};

static const uint8_t config_descriptor[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, TUSB_DESC_TOTAL_LEN, 0, 100),
    /* ESP32-S3 USB is full-speed: 64-byte bulk endpoints */
    TUD_MSC_DESCRIPTOR(ITF_NUM_MSC, 0, EDPT_MSC_OUT, EDPT_MSC_IN, 64),
};

static const char *string_descriptor[] = {
    (const char[]){0x09, 0x04}, /* 0: language = English (US) */
    "evanc",                    /* 1: manufacturer */
    "iPod Classic S3",          /* 2: product */
    "000001",                   /* 3: serial */
};

/* --- Storage ownership --------------------------------------------------- */

static void storage_mount_changed_cb(tinyusb_msc_event_t *event)
{
    bool app_owns = event->mount_changed_data.is_mounted;
    s_app_owns = app_owns;
    ESP_LOGI(TAG, "storage now owned by %s", app_owns ? "app" : "USB host");
    if (s_ownership_cb) {
        s_ownership_cb(app_owns);
    }
}

esp_err_t usb_msc_init(usb_msc_ownership_cb_t cb)
{
    sdmmc_card_t *card = sdcard_get();
    ESP_RETURN_ON_FALSE(card, ESP_ERR_INVALID_STATE, TAG, "SD card not initialized");

    s_ownership_cb = cb;

    const tinyusb_msc_sdmmc_config_t sdmmc_config = {
        .card = card,
        .callback_mount_changed = storage_mount_changed_cb,
        .mount_config = {
            .max_files = 5,
        },
    };
    ESP_RETURN_ON_ERROR(tinyusb_msc_storage_init_sdmmc(&sdmmc_config),
                        TAG, "MSC storage init failed");
    ESP_RETURN_ON_ERROR(tinyusb_msc_storage_mount(USB_MSC_MOUNT_POINT),
                        TAG, "FAT mount failed");
    s_app_owns = true;

    const tinyusb_config_t tusb_cfg = {
        .device_descriptor = &device_descriptor,
        .string_descriptor = string_descriptor,
        .string_descriptor_count = sizeof(string_descriptor) / sizeof(string_descriptor[0]),
        .external_phy = false,
        .configuration_descriptor = config_descriptor,
    };
    ESP_RETURN_ON_ERROR(tinyusb_driver_install(&tusb_cfg), TAG, "TinyUSB install failed");

    ESP_LOGI(TAG, "USB MSC ready, filesystem at %s", USB_MSC_MOUNT_POINT);
    return ESP_OK;
}

bool usb_msc_app_owns_storage(void)
{
    return s_app_owns;
}
