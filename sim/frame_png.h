/*
 * frame_png.h — Minimal, dependency-free PNG writer.
 *
 * Writes 8-bit RGB PNGs using stored (uncompressed) zlib blocks, so it needs
 * neither libpng nor zlib. Good enough for capturing simulator frames that
 * ffmpeg then encodes into the animated demo.
 */
#ifndef FRAME_PNG_H
#define FRAME_PNG_H

#include <stdint.h>

/* rgb points at width*height*3 bytes, row-major, R,G,B order.
 * Returns 0 on success, non-zero on failure. */
int frame_png_write(const char *path, const uint8_t *rgb, int width, int height);

#endif /* FRAME_PNG_H */
