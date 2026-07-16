# esp32_ipod_classic

A modernized iPod: an original iPod shell, clickwheel, and
headphone-jack/hold-switch assemblies, with the Apple mainboard replaced by an
ESP32-S3 (N16R8). Music lives on a microSD card and the device shows up as USB
mass storage when plugged into a computer.

The working prototype targets the **iPod 4th gen (Photo)**: its clickwheel
protocol, breakout boards, and headphone-jack assembly are fully documented
(see [Reference material & credits](#reference-material--credits)), and its
button switches live in the wheel/headphone assemblies rather than on the
Apple mainboard. A port to the iPod Video (5th gen) shell — the preferred
final aesthetic — is planned once the 4th gen prototype works.

Built on **ESP-IDF v5.3** + **ESP-ADF v2.7** (audio pipeline: `fatfs_stream ->
esp_decoder -> i2s_stream`, PCM5102A DAC).

See [docs/HARDWARE.md](docs/HARDWARE.md) for wiring and [TODO.md](TODO.md) for
the roadmap.

## Building

```sh
# One-time setup
git clone --branch v5.3.2 --recurse-submodules https://github.com/espressif/esp-idf.git ~/esp/esp-idf
~/esp/esp-idf/install.sh esp32s3
git clone --branch v2.7 --recurse-submodules https://github.com/espressif/esp-adf.git ~/esp/esp-adf

# Every shell
. ~/esp/esp-idf/export.sh
export ADF_PATH=~/esp/esp-adf

idf.py set-target esp32s3
idf.py build flash monitor
```

## Console (V0.1 control, UART0 @ 115200)

| Command | Action |
|---|---|
| `list [dir]` | List files on the SD card |
| `play /sdcard/track.mp3` | Play a file (MP3/FLAC/WAV/AAC/M4A/OGG) |
| `stop` | Stop playback |
| `sine [seconds]` | 440 Hz DAC smoke test (before first `play` only) |
| `wifi_set <ssid> <pass>` | Store WiFi credentials in NVS |
| `wifi_join` | Connect to WiFi |
| `ota <url>` | Flash new firmware from a URL and reboot |

## Reference material & credits

The `reference/` directory contains third-party research vendored as git
submodules (clone this repo with `--recurse-submodules`, or run
`git submodule update --init` after cloning):

- [Gigahawk/clickwheel_reverse_eng](https://github.com/Gigahawk/clickwheel_reverse_eng) —
  reverse engineering of the iPod clickwheel: FPC pinout, the SPI-like serial
  protocol (clickwheel is the bus *master*: ~55 kHz clock, 32-bit packets,
  CPOL=1, LSB-first), event packet format (button bits, 96-position touch
  location), sleep/init commands, and clicker (piezo) drive notes. Includes
  PulseView/sigrok logic captures. Research was done on a 4th gen (Photo)
  clickwheel — the same generation this project's prototype targets, so it
  applies directly. (5th gen wheels differ: click switches live on the
  mainboard and the FPC has extra pins — relevant to the later Video port.)
- [Gigahawk/clickwheel_sample_firmware](https://github.com/Gigahawk/clickwheel_sample_firmware) —
  working Arduino (ATmega) firmware that reads the clickwheel by bit-banging:
  rising-edge interrupt on SCK shifts bits into a 4-byte packet, packet
  completion detected by matching the fixed header, and a deadband filter
  (default 7 counts) turns raw touch positions into clean clockwise /
  counter-clockwise scroll events. Confirmed on a 4th gen wheel; also links a
  [5th gen breakout board](https://github.com/Gigahawk/clickwheel_breakout_5th_gen)
  design relevant to the planned iPod Video port.

- [Gigahawk/headphone_breakout_4th_gen](https://github.com/Gigahawk/headphone_breakout_4th_gen) —
  KiCad schematics and PCB for a pass-through breakout of the iPod headphone
  jack and remote connector; useful as a probing/interposer reference and for
  the jack/remote pin mapping.

- [Jason Garr — iPod Clickwheel Hack](https://jasongarr.wordpress.com/project-pages/ipod-clickwheel-hack/) —
  the original clickwheel reverse-engineering write-up that the above work
  builds on: 8-pin 4th gen pinout (VBat, SCK, enable pins tied to 3.3 V, data
  with 100 kΩ pull-up, GND), ~60 kHz clock with data sampled on the rising
  edge, 32-bit packets, and AVR interrupt-driven driver code. Notes that the
  5th gen wheel moves to a 14-pin FPC, and that later wheels use different
  controllers entirely (Cypress PSoC / Synaptics T1005 with I2C, SPI, or
  analog interfaces).

Huge thanks to [Gigahawk](https://github.com/Gigahawk) and
[Jason Garr](https://jasongarr.wordpress.com/) for publishing this work.
