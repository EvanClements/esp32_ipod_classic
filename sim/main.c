/*
 * main.c — Headless LVGL capture harness.
 *
 * Boots LVGL with an in-memory display (no SDL, no X server, no GPU), builds
 * the shared UI, then plays the scripted tour at a FIXED timestep — advancing
 * LVGL's clock by a constant amount per frame and snapshotting the framebuffer
 * to a PNG. Because time is driven manually, the captured animation is
 * identical on every machine, so the demo never flickers or drops frames the
 * way a real-time screen recording on a shared CI runner would.
 *
 * Usage: ipod_ui_sim [output_dir]   (default: ./frames)
 */
#include "lvgl.h"
#include "ui.h"
#include "tour.h"
#include "frame_png.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define W 240
#define H 320
#define FRAME_MS 33   /* ~30 fps */

/* XRGB8888 framebuffer that LVGL renders straight into. */
static uint8_t fb[W * H * 4];
static uint8_t rgb[W * H * 3];

static void flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    (void)area;
    (void)px_map; /* full-refresh mode renders into fb directly */
    lv_display_flush_ready(disp);
}

int main(int argc, char **argv)
{
    const char *outdir = (argc > 1) ? argv[1] : "frames";

    lv_init();

    lv_display_t *disp = lv_display_create(W, H);
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_XRGB8888);
    lv_display_set_buffers(disp, fb, NULL, sizeof(fb), LV_DISPLAY_RENDER_MODE_FULL);
    lv_display_set_flush_cb(disp, flush_cb);

    ui_init();

    int nev;
    const tour_event_t *ev = tour_events(&nev);
    int total = tour_total_frames();
    int ei = 0;

    char path[1024];
    for (int frame = 0; frame < total; frame++) {
        /* fire any scripted inputs due this frame */
        while (ei < nev && ev[ei].frame == frame) {
            ui_send_key(ev[ei].key);
            ei++;
        }

        lv_tick_inc(FRAME_MS);
        lv_timer_handler();

        /* XRGB8888 in memory is B,G,R,X (little-endian) -> pack to R,G,B */
        for (int i = 0; i < W * H; i++) {
            rgb[i * 3 + 0] = fb[i * 4 + 2];
            rgb[i * 3 + 1] = fb[i * 4 + 1];
            rgb[i * 3 + 2] = fb[i * 4 + 0];
        }

        snprintf(path, sizeof(path), "%s/frame_%04d.png", outdir, frame);
        if (frame_png_write(path, rgb, W, H) != 0) {
            fprintf(stderr, "failed to write %s\n", path);
            return 1;
        }
    }

    printf("wrote %d frames (%dx%d) to %s\n", total, W, H, outdir);
    return 0;
}
