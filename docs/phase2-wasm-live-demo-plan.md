# Phase 2 — WASM Live Interactive UI Demo (Plan)

> Status: **planned, not yet implemented.** This document is the implementation
> guide for adding a live, clickable web version of the iPod UI on top of the
> Phase 1 headless capture pipeline.

## Goal

Let anyone **play with the real UI in their browser** — scroll the wheel, open
menus, watch Now Playing — with no device and no local build. The same
`ui/ui.c` that runs on the ESP32 and in the headless simulator is compiled to
WebAssembly and rendered to an HTML5 canvas, hosted on GitHub Pages.

This is additive. Phase 1 (the auto-generated README animation) stays exactly
as-is and remains the thing embedded in the README; Phase 2 adds a **"▶ Try it
live" link** next to it.

## Why this is cheap to build now

Phase 1 already did the hard architectural work:

- `ui/ui.c` / `ui/ui.h` are **hardware-independent LVGL** with a single input
  seam, `ui_send_key(ui_key_t)`. Nothing in there needs to change.
- We already know the LVGL version (v9.2.2), the config surface (`lv_conf.h`),
  and the CMake/FetchContent setup.

Phase 2 is a **third build target** for the same UI code: device (Arduino),
native headless (`sim/`), and now web (`web/`). The UI is the constant; only the
display driver, input source, and main loop differ per target.

```
                 ui/ui.c  (shared, hardware-independent)
                    |
      +-------------+-------------------+
      |             |                   |
   ESP32 fw     sim/ (native,        web/ (emscripten,
  (Arduino)     headless PNG)        SDL->canvas, live)   <-- Phase 2
```

## Architecture

Add a `web/` directory that mirrors `sim/` but targets the browser:

```
web/
  main.c            LVGL + SDL display, browser main loop, input export
  lv_conf.h         Web-tuned LVGL config (SDL on, size-optimised)
  shell.html        Page shell: canvas + iPod chrome + on-screen clickwheel
  clickwheel.js     Pointer/drag on the wheel -> exported ui_send_key()
  CMakeLists.txt    Emscripten build (or a build.sh calling emcc)
```

### Display driver

LVGL v9 ships an SDL backend (`LV_USE_SDL`). Under Emscripten, SDL2 is provided
by `-sUSE_SDL=2` and renders to a `<canvas>` via WebGL — so we get a real
display driver with no extra driver code:

```c
lv_display_t *disp = lv_sdl_window_create(UI_W, UI_H);
lv_sdl_mouse_create();      /* pointer -> LVGL (optional, for direct taps) */
lv_sdl_keyboard_create();   /* keyboard -> LVGL group (optional)           */
```

### Main loop

The browser cannot block, so replace `sim/`'s `for` loop with Emscripten's
callback-driven loop (request-animation-frame paced):

```c
static void tick(void *arg) {
    lv_tick_inc(16);          /* ~60 fps; wall-clock time is fine here —   */
    lv_timer_handler();       /* determinism is only needed for capture     */
}
int main(void) {
    lv_init();
    /* create display + input as above */
    ui_init();
    emscripten_set_main_loop_arg(tick, NULL, 0 /*use rAF*/, 1);
    return 0;
}
```

Note the contrast with Phase 1: the headless harness drives a *fixed* timestep
for byte-identical capture; the live demo just needs to feel smooth, so
real-time ticking is correct here.

### Input: the on-screen clickwheel

The signature UX piece. The canvas shows only the screen; the iPod's wheel is
drawn in HTML/SVG around it. `clickwheel.js` translates gestures into the same
four logical inputs the firmware uses:

- **Drag around the ring** → accumulate angle; every N degrees emit
  `UI_KEY_NEXT` (clockwise) or `UI_KEY_PREV` (counter-clockwise).
- **Center button click** → `UI_KEY_SELECT`.
- **"MENU" region tap** → `UI_KEY_MENU`.
- Keyboard fallback: arrow keys / Enter / Backspace map to the same four.

Expose the seam to JS from C:

```c
#include <emscripten.h>
EMSCRIPTEN_KEEPALIVE void web_send_key(int k) { ui_send_key((ui_key_t)k); }
```

```js
// clickwheel.js
const sendKey = Module.cwrap('web_send_key', null, ['number']);
// PREV=0, NEXT=1, SELECT=2, MENU=3  (must match ui_key_t order)
```

Keep the `ui_key_t` enum order the single source of truth; document the numeric
mapping in `clickwheel.js` right next to `cwrap`.

## Build

Two viable routes — pick one:

1. **CMake + Emscripten toolchain** (preferred, matches `sim/`): reuse the same
   FetchContent LVGL declaration; configure with
   `emcmake cmake -S web -B web/build` and build with `cmake --build`. Link
   flags via `target_link_options`.
2. **Thin `build.sh` calling `emcc`** directly: simpler to read, but duplicates
   the LVGL source list. Only choose this if the CMake+emscripten integration
   fights us.

Key `emcc`/link flags:

```
-sUSE_SDL=2
-sALLOW_MEMORY_GROWTH=1
-sEXPORTED_RUNTIME_METHODS=cwrap,ccall
-sEXPORTED_FUNCTIONS=_main,_web_send_key
--shell-file web/shell.html
-Os                        # size matters for a web payload
-o web-dist/index.html
```

### Payload size

LVGL + ThorVG can produce a large `.wasm`. Mitigations, in the web `lv_conf.h`:

- Disable ThorVG / vector graphics if unused (`LV_USE_THORVG_INTERNAL 0`,
  `LV_USE_VECTOR_GRAPHIC 0`).
- Enable only the fonts and widgets the UI actually uses.
- Build `-Os` and enable `-sINITIAL_MEMORY` sizing after measuring.
- Target: keep the gzipped payload comfortably under a few hundred KB. Measure
  early; treat >1 MB gzipped as a signal to trim features.

## Hosting & CI

New workflow `.github/workflows/ui-live.yml`:

```yaml
on:
  push:
    branches: [main]
    paths: ["ui/**", "web/**", ".github/workflows/ui-live.yml"]
  workflow_dispatch:
permissions:
  contents: read
  pages: write
  id-token: write
jobs:
  build-deploy:
    runs-on: ubuntu-latest
    environment: github-pages
    steps:
      - uses: actions/checkout@v4
      - uses: mymindstorm/setup-emsdk@v14
      - run: emcmake cmake -S web -B web/build && cmake --build web/build -j
      - uses: actions/upload-pages-artifact@v3
        with: { path: web-dist }
      - uses: actions/deploy-pages@v4
```

**Prerequisite:** enable GitHub Pages for the repo (Settings → Pages → Source:
GitHub Actions). Live URL will be:
`https://evanclements.github.io/esp32_ipod_classic/`.

Then in `README.md`, add a link beside the Phase 1 animation:

```markdown
[![iPod UI demo](https://.../media/ui-demo.webp)](https://evanclements.github.io/esp32_ipod_classic/)

▶ **[Try the UI live in your browser](https://evanclements.github.io/esp32_ipod_classic/)** — no device needed.
```

## Testing

A lightweight smoke test is enough (the live demo is interactive, so pixel
determinism is out of scope):

- **Local/CI smoke test with Playwright** (Chromium is already available in this
  environment): serve `web-dist/` over http, load the page, wait for the canvas
  to have non-zero size and the module to signal ready, dispatch a few key
  events, and screenshot. Fail if the module doesn't boot or the canvas stays
  blank. Run it as a CI gate on PRs that touch `web/**` or `ui/**`.
- Manual cross-browser pass (Chrome, Firefox, Safari, mobile Safari) for the
  clickwheel gesture feel — hard to automate, worth doing once.

## Milestones

Land these as separate, reviewable PRs:

1. **Boot to canvas.** Emscripten build of the existing UI, keyboard input only,
   run locally via `emrun`. Proves the toolchain and that `ui/` compiles for
   web unmodified. *(Highest-risk step — do it first.)*
2. **iPod chrome + clickwheel.** `shell.html` styling and `clickwheel.js`
   gesture → `web_send_key` mapping. This is the actual "live demo" UX.
3. **Pages deploy.** `ui-live.yml`, enable Pages, wire the README link.
4. **Polish.** Responsive/HiDPI canvas sizing, loading indicator, touch support,
   payload-size trim pass.
5. **(Optional) Playwright smoke test** as a CI gate.
6. **(Optional) Idle auto-play.** When untouched for a few seconds, replay the
   `sim/tour.c` script so the page is never static — reuses the Phase 1 tour
   data.

## Risks & mitigations

| Risk | Mitigation |
| --- | --- |
| CMake + Emscripten integration friction with FetchContent'd LVGL | Fall back to a `build.sh` that invokes `emcc` over an explicit source list |
| WASM payload too large | Trim LVGL features in web `lv_conf.h`, `-Os`, measure gzip early |
| Canvas sizing / HiDPI blurriness | Set canvas backing size = CSS size × `devicePixelRatio`; test on retina |
| GitHub Pages not enabled | One-time repo setting; document it in the milestone-3 PR |
| SDL/emscripten main-loop pitfalls (blocking) | Use `emscripten_set_main_loop_arg`; never call a blocking loop |
| Clickwheel gesture feels wrong on touch vs mouse | Prototype in milestone 2; tune angle-per-tick threshold |

## Effort estimate

Roughly **1–2 focused days**: milestone 1 is a few hours once emscripten is set
up; milestones 2–3 are the bulk; polish is open-ended but optional.

## Open decisions

- **Clickwheel interaction model:** continuous drag-to-rotate vs. tap-arc
  zones. Recommendation: drag-to-rotate (most iPod-authentic), with keyboard as
  the accessible fallback.
- **Idle auto-play (milestone 6):** nice touch, but adds a JS↔C driver. Decide
  after the core demo is up.
- **Payload budget:** set a concrete gzipped-size ceiling once milestone 1
  produces the first `.wasm` to measure against.

## Definition of done

- `web/` builds to `web-dist/` via a single documented command.
- The page loads on GitHub Pages and the UI is fully navigable by wheel/keys.
- README links to the live demo next to the animation.
- CI rebuilds and redeploys on `main` changes to `ui/**` or `web/**`.
- No changes were required to `ui/` — confirming the shared-UI architecture
  held across all three targets.
