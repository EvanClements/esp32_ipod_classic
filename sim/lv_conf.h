/*
 * lv_conf.h — LVGL configuration for the headless simulator build.
 *
 * Only the settings that differ from LVGL's defaults are listed; everything
 * else falls through to lv_conf_internal.h. The device build will have its
 * own lv_conf.h tuned for the ESP32 (PSRAM, display driver, etc.).
 */
#ifndef LV_CONF_H
#define LV_CONF_H

/* 32-bit colour so we can dump clean RGB frames for the recorder. */
#define LV_COLOR_DEPTH 32

/* Plenty of heap for a desktop build. */
#define LV_MEM_SIZE (256 * 1024U)

/* Fonts used by the UI. */
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_22 1
#define LV_FONT_DEFAULT &lv_font_montserrat_14

/* lv_snapshot is handy for still-frame regression tests later. */
#define LV_USE_SNAPSHOT 1

#endif /* LV_CONF_H */
