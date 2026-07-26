#pragma once

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Format a duration given in whole seconds as a display string.
 *
 * Durations under one hour are rendered as "M:SS" (e.g. "3:07"); durations of
 * an hour or more are rendered as "H:MM:SS" (e.g. "1:02:09"). This is the form
 * used for the "Now Playing" running time on the UI.
 *
 * The function is pure (no hardware or ESP-IDF dependencies) so it can be unit
 * tested on the host with the Linux target.
 *
 * @param total_seconds  Non-negative elapsed/total time in seconds.
 * @param buf            Destination buffer.
 * @param size           Size of @p buf in bytes.
 * @return true on success; false if @p buf is NULL, @p size is too small to
 *         hold the result, or @p total_seconds is negative.
 */
bool track_utils_format_time(int total_seconds, char *buf, size_t size);

#ifdef __cplusplus
}
#endif
