# esp32_ipod_classic

A modernized iPod: the original iPod Video (5th gen) shell, clickwheel, and
headphone-jack/hold-switch assemblies, with the Apple mainboard replaced by an
ESP32-S3 (N16R8). Music lives on a microSD card and the device shows up as USB
mass storage when plugged into a computer.

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
