# esp32_ipod_classic
A revitalization using an iPod Video (5th gen) based off of the ESP-32 MCU, built on the shoulders of giants

The plan so far is to create an interface based on the [LittlevGL library](https://lvgl.io/) and a [2.4" 240x320 TFT display](https://www.crystalfontz.com/product/cfaf240320k1024trt-240x320-rgb-tft-lcd). 

I hope to re-use the iPod's ClickWheel using the work of GigaHawk's [ClickWheel Firmware](https://github.com/Gigahawk/clickwheel_sample_firmware), and eventually interface with the existing hold button and headphone jack

I'm not sure at this point whether this will be written in [MicroPython](https://github.com/loboris/MicroPython_ESP32_psRAM_LoBo) or Arduino

Would like USB-C support (both charging and data), and would like to use a Li-Po for the battery

A2DP support for audio out would be nice, and I plan on supporting MP3 and FLAC, along with AAC and OGG

Still need to determine how I will index the files/folders on the [SD card](https://docs.micropython.org/en/latest/library/machine.SDCard.html), but I'm thinking [JSON](https://docs.micropython.org/en/latest/library/ujson.html) to [BTree](https://docs.micropython.org/en/latest/library/btree.html).

On the mention of the SD card, I would like to go with LITTLEFS ([Arduino](https://github.com/lorol/LITTLEFS)|[MicroPython](https://docs.micropython.org/en/latest/reference/filesystem.html))
