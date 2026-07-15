#pragma once

/*
 * Central pin map — see docs/HARDWARE.md for wiring.
 *
 * Off-limits on ESP32-S3 N16R8:
 *   GPIO33-37  octal PSRAM
 *   GPIO19/20  USB D-/D+ (native USB, used for mass storage)
 *   GPIO0/3/45/46  strapping pins
 */

/* microSD — SDMMC host, 4-bit bus */
#define PIN_SD_CLK   14
#define PIN_SD_CMD   15
#define PIN_SD_D0    16
#define PIN_SD_D1    17
#define PIN_SD_D2    18
#define PIN_SD_D3    21

/* PCM5102A DAC — I2S (no MCLK required) */
#define PIN_I2S_BCK  4
#define PIN_I2S_LRCK 5
#define PIN_I2S_DOUT 6

/* Reserved for V0.2+:
 *   display SPI: 7 (SCLK), 8 (MOSI), 9 (DC), 10 (CS), 11 (RST)
 *   clickwheel:  1 (clock), 2 (data)
 *   hold switch: 13
 */
