# Hardware

Replacement mainboard for an iPod 4th gen (Photo) built around an ESP32-S3.
Original shell, clickwheel assembly, and headphone-jack/hold-switch board are
retained; the ESP32-S3 devkit replaces the Apple mainboard. On the 4th gen
the click switches live in the clickwheel assembly (reported in the wheel's
serial packets) and the clicker piezo sits on the headphone board, so no
button hardware needs to be recreated. A later port to the iPod Video
(5th gen) shell is planned; see the README's reference section for what
changes (14-pin wheel FPC, click switches on the mainboard, larger display
aperture).

## Bill of materials (V0.1 bench bring-up)

| Part | Role | Notes |
|---|---|---|
| ESP32-S3 devkit, N16R8 | MCU | 16 MB flash, 8 MB octal PSRAM |
| microSD card + breakout/socket | Music storage | FAT32/exFAT-formatted (FAT32 for V0.1) |
| PCM5102A DAC module | Headphone audio | I2S input; on most modules tie SCK (system clock in) to GND to use the internal PLL |
| iPod 4th gen headphone jack / hold switch board | Audio out + hold + clicker | L / R / GND from DAC output; pin mapping in `reference/headphone_breakout_4th_gen` |
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

The original LCD uses a proprietary interface; it will be replaced with a
modern SPI panel (ST7789/ILI9341-class) behind the original front glass.
The 4th gen (Photo) display aperture is ~2.0" (original panel 220x176), so
the target is a 2.0" 320x240 ST7789 IPS module. (The 5th gen port can move
to a 2.8" panel — its window is larger.)

## Clickwheel (V0.2)

The original Apple clickwheel controller speaks a reverse-engineered serial
protocol — the wheel is the bus master: ~55–60 kHz clock, 32-bit packets
(header `0x35`, button bits, 96-position touch location, touch-active flag),
LSB-first, pull-ups required on clock and data. Full protocol notes and
working reference code live in `reference/clickwheel_reverse_eng` and
`reference/clickwheel_sample_firmware`. It will get its own component with an
interrupt-driven (bit-bang) driver on the reserved GPIOs.
