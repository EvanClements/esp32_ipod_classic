#include "track_utils.h"

#include <stdio.h>

bool track_utils_format_time(int total_seconds, char *buf, size_t size)
{
    if (buf == NULL || size == 0 || total_seconds < 0) {
        return false;
    }

    const int hours = total_seconds / 3600;
    const int minutes = (total_seconds % 3600) / 60;
    const int seconds = total_seconds % 60;

    int written;
    if (hours > 0) {
        written = snprintf(buf, size, "%d:%02d:%02d", hours, minutes, seconds);
    } else {
        written = snprintf(buf, size, "%d:%02d", minutes, seconds);
    }

    /* snprintf returns the length it *would* have written; a value >= size
     * means the result was truncated and therefore invalid. */
    return written > 0 && (size_t)written < size;
}
