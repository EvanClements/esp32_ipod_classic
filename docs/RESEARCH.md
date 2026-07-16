# Research notes

Third-party reverse-engineering research this project builds on. The repos
below are vendored as git submodules under `reference/` (clone with
`--recurse-submodules`, or run `git submodule update --init` after cloning).

## Sources

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

## Key findings for this project

### Clickwheel protocol (4th gen, verified by two independent sources)

- Wheel is the bus **master** — the MCU only listens (and can answer on MISO
  when CFG1 = 0). ~55–60 kHz clock, 32-bit packets, CPOL=1, LSB-first,
  pull-ups required on clock and data.
- Packet: byte 0 header `0x35`; byte 1 button bits (Menu/Play/Prev/Next/
  Center); byte 2 touch position `0x00`–`0xBE` (96 positions, steps of 2,
  clockwise from Menu); byte 3 touch-active flag (`0x80` finger down, `0x00`
  ignore position).
- CFG1 pin: low = continuous ~1.465 kHz packets (`0xFF` when idle) and the
  CPU may send commands; high = event-only packets (~67 Hz max).
- Commands (CFG1 = 0): sleep `95 02 00 C0`; wake handshake `1D 01 00 C0`
  answered by `75 04 02 00`. Hold switch cuts wheel power; on release the
  wheel emits `0xAA` bytes until ready.
- The two sources disagree on sample edge (falling vs rising); both work
  because the header bits are constant, so either edge syncs.

### Driver approach

- Bit-banging is proven: rising-edge interrupt on SCK, shift bits into a
  4-byte buffer, detect completion by matching the fixed header — no
  chip-select or start-pulse handling needed. 32 bits at ~55 kHz is slow
  enough that GPIO interrupts beat fighting SPI-slave DMA for 4-byte frames.
- Scroll decoding: compare touch position to previous, accumulate into a
  deadband counter (default 7), emit a scroll event past the threshold, then
  reset the counter to half. Tune the constant for scroll feel.
- Clicker (piezo) needs a MOSFET for full volume — bare GPIO drive is too
  weak; a P-channel part is recommended since the clicker shares the iPod's
  digital ground.

### 4th gen vs 5th gen (Video)

- 4th gen: 8-pin wheel FPC, click switches inside the wheel/headphone
  assemblies (reported in wheel packets), clicker on the headphone board,
  ~2.0" display aperture. Everything above applies directly.
- 5th gen: 14-pin wheel FPC (pinout not fully documented — Gigahawk's
  breakout is the best starting point but untested by its own author), click
  switches on the Apple mainboard (must be recreated as a custom dome-switch
  PCB), larger ~2.5" display aperture.
- 6th gen and later wheels use different controllers entirely (Cypress PSoC /
  Synaptics T1005; I2C, SPI, or analog) — out of scope.
