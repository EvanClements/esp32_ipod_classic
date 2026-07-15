## Roadmap

### V0.1 — Console
Goal: bring up audio pipeline and prove hardware works, controlled via serial console (no physical UI yet).

- [x] AUDIO: play MP3/FLAC/WAV/AAC/M4A/OGG on SD card
  - Success: file plays end-to-end through PCM5102A DAC, no dropouts
- [x] Output via I2S
  - Success: clean audio at 44.1/48kHz, verified by ear + scope
- [x] OTA updates
  - Success: flash new firmware from URL over WiFi, reboots into new image
- [x] USB MSC (SD card mounts as mass storage on host)
  - Success: drag-and-drop files from PC, ejects safely

### V0.2 — Navigation
Goal: user can browse and select tracks without touching console commands.

- [ ] UI: simple text lists, "now playing" text
  - Success: menu renders in <200ms, text readable on screen at arm's length
- [ ] CTRL: navigate with Clickwheel
  - Success: scroll+select registers reliably, no missed clicks across a 20-item list

### V0.3 — Library
Goal: browsing is driven by a real track database instead of raw file listings.

- [ ] DB: manually enter files and file names
  - Success: DB entries persist across reboot, correctly map to files on SD card
- [ ] UI: load UI using DB
  - Success: menu list populates from DB query, matches manually-entered entries exactly

### V0.4 — Now Playing
Goal: user can control and monitor active playback from the device itself.

- [ ] UI: Now Playing shows running time
  - Success: elapsed time updates at least once per second, matches actual playback position within 1s
- [ ] CTRL: Play/Pause
  - Success: single click toggles playback state within 100ms, no audio glitch on resume

### V0.5 — Volume
Goal: user can adjust playback volume without console access.

- [ ] CTRL+UI: volume control on Now Playing
  - Success: clickwheel scroll adjusts volume in real time, on-screen level indicator matches actual output level

### V0.6 — Bluetooth
Goal: audio can be routed to a wireless speaker/headphones instead of the wired DAC.

- [ ] Bluetooth A2DP Source support, with switching between I2S and BT output
  - Success: pairs with a standard BT sink, switching output mid-playback causes no crash and resumes audio within 2s

### V0.7 — Status Bar
Goal: user can see device/connectivity state at a glance.

- [ ] UI: status indicators for BT/WiFi/Clock on top bar
  - Success: each indicator reflects true state within 1s of a state change (e.g. BT connect/disconnect)
