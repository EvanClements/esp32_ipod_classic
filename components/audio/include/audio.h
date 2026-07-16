#pragma once

#include "esp_err.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Build the ADF playback pipeline:
 *   fatfs_stream (reader) -> esp_decoder (auto-detect) -> i2s_stream (writer)
 * and start the event-listener task. Call once after the SD card filesystem
 * is available.
 */
esp_err_t audio_init(void);

/** Play one file from the mounted filesystem, e.g. "/sdcard/track.flac".
 *  Stops any current playback first. */
esp_err_t audio_play(const char *path);

/** Stop playback if running. Safe to call when idle. Blocks until stopped. */
esp_err_t audio_stop(void);

bool audio_is_playing(void);

/** 440 Hz stereo sine to the DAC for `seconds` — hardware smoke test.
 *  Only allowed while audio_init() has not run or playback is stopped;
 *  uses its own temporary I2S channel. */
esp_err_t audio_sine_test(int seconds);

#ifdef __cplusplus
}
#endif
