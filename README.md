# esp32_ipod_classic
A revitalization using an iPod Video (5th gen) based off of the ESP-32 MCU, built on the shoulders of giants

The plan so far is to create an interface based on the [LittlevGL library](https://lvgl.io/) and a [2.4" 240x320 TFT display](https://www.crystalfontz.com/product/cfaf240320k1024trt-240x320-rgb-tft-lcd). 

I hope to re-use the iPod's ClickWheel using the work of GigaHawk's [ClickWheel Firmware](https://github.com/Gigahawk/clickwheel_sample_firmware), and eventually interface with the existing hold button and headphone jack

~~I'm not sure at this point whether this will be written in [MicroPython](https://github.com/loboris/MicroPython_ESP32_psRAM_LoBo) or Arduino~~
I'm hoping to work directly in C/C++ through ESP-IDF, as Arduino and MicroPython are too abstracted, and, if I'm going to learn a programming language, why not go straight for the gold with C/C++?

Would like USB-C support (both charging and data), and would like to use a Li-Po for the battery

A2DP support for audio out would be nice, and I plan on supporting MP3 and FLAC, along with AAC and OGG

~~Still need to determine how I will index the files/folders on the [SD card](https://docs.micropython.org/en/latest/library/machine.SDCard.html), but I'm thinking [JSON](https://docs.micropython.org/en/latest/library/ujson.html) to [BTree](https://docs.micropython.org/en/latest/library/btree.html).~~
Or we could just go with [a SQLite3 database](https://github.com/siara-cc/esp32-idf-sqlite3)

On the mention of the SD card, I would like to go with LITTLEFS ([Arduino](https://github.com/lorol/LITTLEFS)|[MicroPython](https://docs.micropython.org/en/latest/reference/filesystem.html)) for the filesystem. I think I will have to read the ID3 tags on input and then export the necessary information to JSON which will then be read by the ESP32 at init (maybe save time with a check of when it was last updated?) and that will be loaded into BTree DB?

At this point, I have determined the following:
* I'm going to use PlatformIO in VS Code as my IDE
* I will be using ESP-IDF
* I will be coding in C/C++
* The UI will be programmed using the LittlevGL library
* This will be based off of the ESP32-WROOM module
* My eventual goal is a working drop in iPod replacement board that can replace the internals of the iPod
  * Flash storage with larger capacity
  * Larger battery capacity for longer runtime
  * Similar UI
  * Working ClickWheel support out of the box
  * Bluetooth A2DP support for headphones, car stereos, and speakers
  * WiFi for transferring files to and from the device
  * WiFi for Internet Radio (Spotify?)
  * Support for MP3, FLAC, AAC, OGG (Lyrics? AlbumArt? Other Codecs?)

## To Do
- [ ] Confirm A2DP support by manually connecting to BT headphones
- [ ] Confirm MP3 support by playing a MP3 file over BT
- [ ] Confirm OGG support by playing an OGG file over BT
- [ ] Confirm FLAC support by playing a FLAC file over BT
- [ ] Confirm AAC support by playing an AAC file over BT
- [ ] Set up LITTLEFS partitions (Can we use ESP-IDFs VFS?)
- [ ] Set up SQLite3 database for metadata and filepath storage
- [ ] Find out the largest SD card supported by ESP32 and filesystem
