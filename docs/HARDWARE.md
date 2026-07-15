# Hardware

Replacement mainboard for an iPod Video (5th gen) built around an ESP32-S3.
Original shell, clickwheel assembly, and headphone-jack/hold-switch flex are
retained; the ESP32-S3 devkit replaces the Apple mainboard.

## Bill of materials (V0.1 bench bring-up)

| Part | Role | Notes |
|---|---|---|
| ESP32-S3 devkit, N16R8 | MCU | 16 MB flash, 8 MB octal PSRAM |
| microSD card + breakout/socket | Music storage | FAT32/exFAT-formatted (FAT32 for V0.1) |
| PCM5102A DAC module | Headphone audio | I2S input; on most modules tie SCK (system clock in) to GND to use the internal PLL |
| iPod 5g headphone jack / hold switch flex | Audio out + hold | L / R / GND from DAC output |
| LiPo cell + TP4056-class charger | Power (later) | Bench bring-up can run from USB |

## Pin map

All peripherals are routed through the GPIO matrix; pins live in
`components/board/include/board_pins.h`.

### Reserved / off-limits on ESP32-S3 N16R8

| GPIO | Why |
|---|---|
| 33–37 | Octal PSRAM (N16R8) |
| 19, 20 | Native USB D- / D+ (mass storage to the PC) |
| 0, 3, 45, 46 | Strapping pins |
| 26–32 | SPI flash |

### microSD — SDMMC host, 4-bit bus

| Signal | GPIO |
|---|---|
| CLK | 14 |
| CMD | 15 |
| D0 | 16 |
| D1 | 17 |
| D2 | 18 |
| D3 | 21 |

Internal pull-ups are enabled in firmware, but real 10 kΩ pull-ups on
CMD/D0-D3 are recommended for reliable high-speed operation.

### PCM5102A — I2S

| Signal | GPIO | PCM5102A pin |
|---|---|---|
| BCK | 4 | BCK |
| LRCK | 5 | LCK |
| DOUT | 6 | DIN |
| — | — | SCK → GND (internal PLL) |

### Reserved for V0.2+

| Function | GPIO |
|---|---|
| Display SPI (SCLK/MOSI/DC/CS/RST) | 7 / 8 / 9 / 10 / 11 |
| Clickwheel (clock / data) | 1 / 2 |
| Hold switch | 13 |

## USB

Native USB OTG (GPIO19/20) enumerates as a USB mass-storage device exposing
the raw SD card. While a host has claimed the storage, firmware unmounts the
filesystem and playback is blocked; on unplug the card is remounted
automatically. The serial console runs on UART0 (the devkit's USB-UART
bridge), so console and MSC can be used at the same time.

## Display (V0.2 decision)

The original 5th-gen LCD uses a proprietary interface; it will be replaced
with a 2.8" 320x240 SPI panel (ST7789/ILI9341-class) behind the original
front glass.

## Clickwheel (V0.2)

The original Apple clickwheel controller speaks a reverse-engineered 2-wire
serial protocol (clock + data, 32-bit packets carrying wheel position and
button states). It will get its own component and interrupt-driven driver.
