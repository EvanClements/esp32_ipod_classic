/*
 * ui.h — Hardware-independent iPod-style UI built on LVGL.
 *
 * This layer knows nothing about the ESP32, the clickwheel, I2S, or the SD
 * card. It draws into whatever LVGL display is active and receives input
 * through ui_send_key(). On the device, the clickwheel driver calls
 * ui_send_key(); in the simulator, the scripted tour calls the same function.
 * Keeping this seam clean is what lets the exact same UI code run on-device
 * and in headless CI.
 */
#ifndef UI_H
#define UI_H

#ifdef __cplusplus
extern "C" {
#endif

/* Logical input events. The clickwheel maps rotation + center button to
 * these; the simulator tour emits them directly. */
typedef enum {
    UI_KEY_PREV,   /* wheel counter-clockwise: move selection up   */
    UI_KEY_NEXT,   /* wheel clockwise:         move selection down */
    UI_KEY_SELECT, /* center button:           drill in / play     */
    UI_KEY_MENU,   /* menu button:             go back             */
} ui_key_t;

/* Build the UI and show the root menu. Call once after LVGL is initialised
 * and a display exists. */
void ui_init(void);

/* Feed one logical input event to the UI. Safe to call between
 * lv_timer_handler() ticks. */
void ui_send_key(ui_key_t key);

#ifdef __cplusplus
}
#endif

#endif /* UI_H */
