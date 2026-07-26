#include "frame_png.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- CRC32 (PNG chunks) -------------------------------------------------- */
static uint32_t crc32_update(uint32_t crc, const uint8_t *buf, size_t len)
{
    static uint32_t table[256];
    static int have_table = 0;
    if (!have_table) {
        for (uint32_t n = 0; n < 256; n++) {
            uint32_t c = n;
            for (int k = 0; k < 8; k++)
                c = (c & 1) ? 0xedb88320u ^ (c >> 1) : (c >> 1);
            table[n] = c;
        }
        have_table = 1;
    }
    crc ^= 0xffffffffu;
    for (size_t i = 0; i < len; i++)
        crc = table[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);
    return crc ^ 0xffffffffu;
}

/* ---- Adler32 (zlib) ------------------------------------------------------ */
static uint32_t adler32(const uint8_t *data, size_t len)
{
    uint32_t a = 1, b = 0;
    for (size_t i = 0; i < len; i++) {
        a = (a + data[i]) % 65521u;
        b = (b + a) % 65521u;
    }
    return (b << 16) | a;
}

/* ---- little writer helpers ----------------------------------------------- */
static void put_be32(FILE *f, uint32_t v)
{
    uint8_t b[4] = { (uint8_t)(v >> 24), (uint8_t)(v >> 16),
                     (uint8_t)(v >> 8),  (uint8_t)v };
    fwrite(b, 1, 4, f);
}

static void write_chunk(FILE *f, const char *type, const uint8_t *data, size_t len)
{
    put_be32(f, (uint32_t)len);
    fwrite(type, 1, 4, f);
    if (len) fwrite(data, 1, len, f);

    /* CRC is over the type + data as one run; crc32_update xors in/out per
     * call, so compute it over a single concatenated buffer. */
    uint8_t *tmp = (uint8_t *)malloc(4 + len);
    memcpy(tmp, type, 4);
    if (len) memcpy(tmp + 4, data, len);
    uint32_t crc = crc32_update(0, tmp, 4 + len);
    free(tmp);
    put_be32(f, crc);
}

int frame_png_write(const char *path, const uint8_t *rgb, int width, int height)
{
    FILE *f = fopen(path, "wb");
    if (!f) return 1;

    static const uint8_t sig[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };
    fwrite(sig, 1, 8, f);

    /* IHDR */
    uint8_t ihdr[13];
    ihdr[0] = (uint8_t)(width >> 24);  ihdr[1] = (uint8_t)(width >> 16);
    ihdr[2] = (uint8_t)(width >> 8);   ihdr[3] = (uint8_t)width;
    ihdr[4] = (uint8_t)(height >> 24); ihdr[5] = (uint8_t)(height >> 16);
    ihdr[6] = (uint8_t)(height >> 8);  ihdr[7] = (uint8_t)height;
    ihdr[8]  = 8;   /* bit depth   */
    ihdr[9]  = 2;   /* colour type: truecolour RGB */
    ihdr[10] = 0;   /* compression */
    ihdr[11] = 0;   /* filter      */
    ihdr[12] = 0;   /* interlace   */
    write_chunk(f, "IHDR", ihdr, sizeof(ihdr));

    /* Raw filtered image: each row prefixed with filter byte 0 (None). */
    size_t row_bytes = (size_t)width * 3;
    size_t raw_len   = (row_bytes + 1) * (size_t)height;
    uint8_t *raw = (uint8_t *)malloc(raw_len);
    if (!raw) { fclose(f); return 2; }
    for (int y = 0; y < height; y++) {
        uint8_t *dst = raw + (size_t)y * (row_bytes + 1);
        *dst++ = 0;
        memcpy(dst, rgb + (size_t)y * row_bytes, row_bytes);
    }

    /* zlib stream: 2-byte header + stored deflate blocks + adler32. */
    size_t max_zlen = 2 + raw_len + 5 * (raw_len / 65535 + 1) + 4;
    uint8_t *z = (uint8_t *)malloc(max_zlen);
    if (!z) { free(raw); fclose(f); return 3; }
    size_t zp = 0;
    z[zp++] = 0x78;   /* CMF */
    z[zp++] = 0x01;   /* FLG (no dict, fastest) */

    size_t off = 0;
    while (off < raw_len) {
        size_t block = raw_len - off;
        if (block > 65535) block = 65535;
        uint8_t bfinal = (off + block >= raw_len) ? 1 : 0;
        z[zp++] = bfinal;                       /* stored block, BTYPE=00 */
        z[zp++] = (uint8_t)(block & 0xff);
        z[zp++] = (uint8_t)((block >> 8) & 0xff);
        uint16_t nlen = (uint16_t)~block;
        z[zp++] = (uint8_t)(nlen & 0xff);
        z[zp++] = (uint8_t)((nlen >> 8) & 0xff);
        memcpy(z + zp, raw + off, block);
        zp += block;
        off += block;
    }
    uint32_t ad = adler32(raw, raw_len);
    z[zp++] = (uint8_t)(ad >> 24); z[zp++] = (uint8_t)(ad >> 16);
    z[zp++] = (uint8_t)(ad >> 8);  z[zp++] = (uint8_t)ad;

    write_chunk(f, "IDAT", z, zp);
    write_chunk(f, "IEND", NULL, 0);

    free(z);
    free(raw);
    fclose(f);
    return 0;
}
