/*
 * Custom board header. ESP-ADF's esp_peripherals includes "board.h"
 * unconditionally; this board has no codec chip to set up (PCM5102A is
 * configuration-free), so there is nothing here beyond the pin lookups.
 */
#ifndef _AUDIO_BOARD_H_
#define _AUDIO_BOARD_H_

#include "board_pins_config.h"

/* Required at compile time by ADF audio_hal codec drivers; unused at runtime
 * (no codec chip on this board — the PCM5102A has no control interface). */
#define BOARD_PA_GAIN (0)

/* SD card pins for ADF's esp_peripherals sdcard glue — kept in sync with
 * board_pins.h (this project mounts the card itself via components/sdcard,
 * but the macros must exist and may as well be correct). */
#define ESP_SD_PIN_CLK 14
#define ESP_SD_PIN_CMD 15
#define ESP_SD_PIN_D0  16
#define ESP_SD_PIN_D1  17
#define ESP_SD_PIN_D2  18
#define ESP_SD_PIN_D3  21
#define ESP_SD_PIN_D4  -1
#define ESP_SD_PIN_D5  -1
#define ESP_SD_PIN_D6  -1
#define ESP_SD_PIN_D7  -1
#define ESP_SD_PIN_CD  -1
#define ESP_SD_PIN_WP  -1

int8_t get_sdcard_open_file_num_max(void);

#endif
