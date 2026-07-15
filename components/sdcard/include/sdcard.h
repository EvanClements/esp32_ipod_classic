#pragma once

#include "esp_err.h"
#include "sdmmc_cmd.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the SDMMC host (4-bit bus) and probe the card.
 * Does NOT mount a filesystem — the USB MSC layer owns mounting so the
 * card can be handed to the host as a raw block device.
 */
esp_err_t sdcard_init(void);

/** Card handle, or NULL if sdcard_init() has not succeeded. */
sdmmc_card_t *sdcard_get(void);

#ifdef __cplusplus
}
#endif
