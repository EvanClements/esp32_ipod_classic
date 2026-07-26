# Testing

Automated checks run on every push and pull request via
[`.github/workflows/build.yml`](.github/workflows/build.yml), so each change is
verified for both **confirmation** (does it still build?) and **regression**
(do the tests still pass?).

## Test layers

The project is set up to grow into three layers, cheapest and fastest first.

| Layer | What it checks | In CI now? |
| --- | --- | --- |
| **Firmware build** | The project compiles for the ESP32 target | ✅ `firmware-build` job |
| **Native unit tests** | Pure logic, compiled with GCC and run on the CI host in milliseconds | ✅ `host-unit-tests` job |
| **Simulation / integration** | The real firmware boots and behaves (serial, input, peripherals) | ⏳ ready to add — see below |

## Native unit tests (host / Linux target)

ESP-IDF's [Linux target](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/host-apps.html)
lets us compile hardware-independent components on the host and run them with
[Unity](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/unit-tests.html).
No board, simulator, or secrets required.

- Component under test: [`components/track_utils`](components/track_utils)
- Host test app: [`components/track_utils/test`](components/track_utils/test)

Keep unit-testable logic (parsing, playlist/DB ordering, formatting, state
machines) in plain C components that don't depend on ESP-IDF drivers, then add a
matching test in the `test/` app.

### Run locally

With ESP-IDF exported (`. $IDF_PATH/export.sh`):

```bash
cd components/track_utils/test
idf.py --preview set-target linux build
./build/track_utils_host_test.elf
```

The process exits non-zero if any assertion fails, so it works directly in CI.

## Adding the simulation layer later

Because the project is now ESP-IDF (not Arduino), the integration layer has two
options that both reuse the firmware build:

- **QEMU** — Espressif's emulator. Needs **no account or token**; good default
  for booting the firmware and asserting on serial output in CI.
- **[Wokwi CI](https://docs.wokwi.com/wokwi-ci/getting-started)** — cloud
  simulator with richer peripherals (displays, buttons/clickwheel, I2S).
  Requires a free `WOKWI_CLI_TOKEN` repository secret plus `wokwi.toml` and
  `diagram.json`.

Both can be driven with
[`pytest-embedded`](https://github.com/espressif/pytest-embedded) using
`dut.expect(...)` assertions — select the backend with
`--embedded-services idf,qemu` or `--embedded-services idf,wokwi`. This is the
natural place to add checks for the V0.1+ audio/UI/clickwheel milestones as that
code lands.
