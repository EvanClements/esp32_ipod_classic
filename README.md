# esp32_ipod_classic

A revitalization using an iPod Video (5th gen) based off of the ESP-32 MCU

## UI Preview

The animation below is generated automatically from the actual LVGL UI code on
every merge to `main` that touches the UI — no device required. It renders a
scripted "tour" of the interface headlessly in CI. See
[Development → UI simulator](#ui-simulator) for how it works.

![iPod UI demo](https://raw.githubusercontent.com/EvanClements/esp32_ipod_classic/media/ui-demo.webp)

<!-- If the WebP above doesn't animate in your viewer, the GIF fallback lives at
     .../media/ui-demo.gif on the same branch. -->

## Development

### UI simulator

The UI is written in **LVGL** and kept strictly hardware-independent (see
`ui/`), so the exact same code runs on the ESP32 and on a plain Linux host.
That lets CI build the UI and record how it looks and behaves without any
hardware in the loop.

```
ui/                 Hardware-independent LVGL UI (shared by device + sim)
  ui.h / ui.c       Screens, navigation, and the ui_send_key() input seam
sim/                Host-only capture harness
  main.c            Headless LVGL display -> framebuffer -> PNG per frame
  tour.c / tour.h   The scripted demo (a table of {frame, key} events)
  frame_png.*       Tiny dependency-free PNG writer (no libpng/zlib needed)
  lv_conf.h         LVGL config for the host build
  encode.sh         Frames -> animated WebP + GIF (uses ffmpeg)
  CMakeLists.txt    Fetches LVGL and builds `ipod_ui_sim`
```

Build and run it locally:

```bash
cmake -S sim -B sim/build -DCMAKE_BUILD_TYPE=Release
cmake --build sim/build -j
./sim/build/ipod_ui_sim frames        # writes frames/frame_0000.png ...
./sim/encode.sh frames out            # writes out/ui-demo.webp and .gif
```

**Why it's reproducible:** the harness drives LVGL's clock by a fixed step per
frame instead of screen-recording in real time, so the captured animation
(including LVGL's easing and progress animations) is byte-for-byte identical on
every machine and never drops frames under CI load. The same PNG frames can
later back visual-regression tests.

The clickwheel maps to four logical inputs — `PREV`, `NEXT`, `SELECT`, `MENU` —
fed through `ui_send_key()`. On the device the clickwheel driver calls it; in
the simulator the scripted tour calls it. To change what the demo shows, edit
the event table in `sim/tour.c`.

### Firmware build

The ESP32 firmware is built with the Arduino CLI (see
`.github/workflows/main.yml`).

### Roadmap: live interactive demo

A future phase compiles the same `ui/` code to WebAssembly for a clickable,
in-browser version of the UI hosted on GitHub Pages. The implementation plan
lives in [`docs/phase2-wasm-live-demo-plan.md`](docs/phase2-wasm-live-demo-plan.md).
