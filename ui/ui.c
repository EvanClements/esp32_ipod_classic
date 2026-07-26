/*
 * ui.c — A small but real iPod-classic-style UI on LVGL v9.
 *
 * Screens: a root Menu, a Songs list, and a Now Playing view. Selection moves
 * with a smooth animated highlight (the classic blue bar), and Now Playing
 * animates a progress bar so the recorded demo actually shows motion.
 *
 * Everything here is portable LVGL — no hardware headers — so the device
 * firmware and the CI simulator share this exact file.
 */
#include "ui.h"
#include "lvgl.h"

#include <stdio.h>

/* iPod 5G portrait menu geometry. */
#define UI_W 240
#define UI_H 320
#define STATUSBAR_H 26
#define ROW_H 34

/* ---- iPod-ish palette ---------------------------------------------------- */
#define COL_BG        lv_color_hex(0xffffff)
#define COL_TEXT      lv_color_hex(0x101010)
#define COL_SEL_TOP   lv_color_hex(0x4a90ff)
#define COL_SEL_BOT   lv_color_hex(0x1560d8)
#define COL_SEL_TEXT  lv_color_hex(0xffffff)
#define COL_BAR_TOP   lv_color_hex(0xdfe4ea)
#define COL_BAR_BOT   lv_color_hex(0xb6bec9)
#define COL_ACCENT    lv_color_hex(0x1560d8)

typedef enum { SCREEN_MENU, SCREEN_SONGS, SCREEN_NOWPLAYING } screen_id_t;

/* A simple list-style screen (Menu and Songs both use this shape). */
typedef struct {
    lv_obj_t   *screen;
    lv_obj_t   *list;       /* scrollable container of rows */
    lv_obj_t   *highlight;  /* the animated selection bar   */
    lv_obj_t  **rows;
    const char *const *items;
    int         count;
    int         sel;
} list_view_t;

static struct {
    screen_id_t   current;
    list_view_t   menu;
    list_view_t   songs;

    /* Now Playing */
    lv_obj_t     *np_screen;
    lv_obj_t     *np_bar;
    lv_obj_t     *np_elapsed;
    lv_obj_t     *np_remain;
} S;

static const char *const MENU_ITEMS[] = {
    "Music", "Photos", "Videos", "Extras", "Settings", "Shuffle Songs",
};
static const char *const SONG_ITEMS[] = {
    "Baba O'Riley", "1979", "Such Great Heights", "Reptilia",
    "Karma Police", "Time to Pretend", "Electioneering",
};

/* ---- shared styling ------------------------------------------------------ */

static void make_statusbar(lv_obj_t *screen, const char *title)
{
    lv_obj_t *bar = lv_obj_create(screen);
    lv_obj_remove_style_all(bar);
    lv_obj_set_size(bar, UI_W, STATUSBAR_H);
    lv_obj_set_pos(bar, 0, 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(bar, COL_BAR_TOP, 0);
    lv_obj_set_style_bg_grad_color(bar, COL_BAR_BOT, 0);
    lv_obj_set_style_bg_grad_dir(bar, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_border_width(bar, 0, 0);
    lv_obj_set_style_border_side(bar, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_width(bar, 1, 0);
    lv_obj_set_style_border_color(bar, lv_color_hex(0x8a94a0), 0);

    lv_obj_t *label = lv_label_create(bar);
    lv_label_set_text(label, title);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(label, COL_TEXT, 0);
    lv_obj_center(label);

    /* A tiny battery glyph on the right, purely cosmetic. */
    lv_obj_t *batt = lv_label_create(bar);
    lv_label_set_text(batt, LV_SYMBOL_BATTERY_FULL);
    lv_obj_set_style_text_color(batt, COL_TEXT, 0);
    lv_obj_align(batt, LV_ALIGN_RIGHT_MID, -8, 0);
}

/* Move the highlight bar to the selected row with a smooth animation and keep
 * the row visible. Also recolours row text for the selected/deselected rows. */
static void anim_y_cb(void *obj, int32_t v) { lv_obj_set_y((lv_obj_t *)obj, v); }

static void list_apply_selection(list_view_t *lv, int new_sel)
{
    if (new_sel < 0) new_sel = 0;
    if (new_sel >= lv->count) new_sel = lv->count - 1;

    /* de-tint old row, tint new row */
    lv_obj_set_style_text_color(lv->rows[lv->sel], COL_TEXT, 0);
    lv_obj_set_style_text_color(lv->rows[new_sel], COL_SEL_TEXT, 0);
    lv->sel = new_sel;

    int target_y = new_sel * ROW_H;

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, lv->highlight);
    lv_anim_set_values(&a, lv_obj_get_y(lv->highlight), target_y);
    lv_anim_set_time(&a, 140);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_set_exec_cb(&a, anim_y_cb);
    lv_anim_start(&a);

    lv_obj_scroll_to_view(lv->rows[new_sel], LV_ANIM_ON);
}

static void build_list_view(list_view_t *lv, const char *title,
                            const char *const *items, int count,
                            bool chevron)
{
    lv->items = items;
    lv->count = count;
    lv->sel   = 0;

    lv->screen = lv_obj_create(NULL);
    lv_obj_remove_style_all(lv->screen);
    lv_obj_set_style_bg_opa(lv->screen, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(lv->screen, COL_BG, 0);

    make_statusbar(lv->screen, title);

    lv->list = lv_obj_create(lv->screen);
    lv_obj_remove_style_all(lv->list);
    lv_obj_set_size(lv->list, UI_W, UI_H - STATUSBAR_H);
    lv_obj_set_pos(lv->list, 0, STATUSBAR_H);
    lv_obj_set_style_bg_opa(lv->list, LV_OPA_TRANSP, 0);
    lv_obj_set_scrollbar_mode(lv->list, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(lv->list, LV_DIR_VER);

    /* selection highlight sits behind the row labels */
    lv->highlight = lv_obj_create(lv->list);
    lv_obj_remove_style_all(lv->highlight);
    lv_obj_set_size(lv->highlight, UI_W, ROW_H);
    lv_obj_set_pos(lv->highlight, 0, 0);
    lv_obj_set_style_bg_opa(lv->highlight, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(lv->highlight, COL_SEL_TOP, 0);
    lv_obj_set_style_bg_grad_color(lv->highlight, COL_SEL_BOT, 0);
    lv_obj_set_style_bg_grad_dir(lv->highlight, LV_GRAD_DIR_VER, 0);

    lv->rows = lv_malloc(sizeof(lv_obj_t *) * count);
    for (int i = 0; i < count; i++) {
        lv_obj_t *row = lv_obj_create(lv->list);
        lv_obj_remove_style_all(row);
        lv_obj_set_size(row, UI_W, ROW_H);
        lv_obj_set_pos(row, 0, i * ROW_H);
        lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_side(row, LV_BORDER_SIDE_BOTTOM, 0);
        lv_obj_set_style_border_width(row, 1, 0);
        lv_obj_set_style_border_color(row, lv_color_hex(0xe2e2e2), 0);

        lv_obj_t *label = lv_label_create(row);
        lv_label_set_text(label, items[i]);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(label, i == 0 ? COL_SEL_TEXT : COL_TEXT, 0);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 12, 0);
        lv->rows[i] = label;

        if (chevron) {
            lv_obj_t *chev = lv_label_create(row);
            lv_label_set_text(chev, LV_SYMBOL_RIGHT);
            lv_obj_set_style_text_color(chev, lv_color_hex(0x9aa2ac), 0);
            lv_obj_align(chev, LV_ALIGN_RIGHT_MID, -10, 0);
        }
    }
}

/* ---- Now Playing --------------------------------------------------------- */

static void build_now_playing(void)
{
    S.np_screen = lv_obj_create(NULL);
    lv_obj_remove_style_all(S.np_screen);
    lv_obj_set_style_bg_opa(S.np_screen, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(S.np_screen, COL_BG, 0);

    make_statusbar(S.np_screen, "Now Playing");

    /* album-art placeholder */
    lv_obj_t *art = lv_obj_create(S.np_screen);
    lv_obj_remove_style_all(art);
    lv_obj_set_size(art, 150, 150);
    lv_obj_align(art, LV_ALIGN_TOP_MID, 0, STATUSBAR_H + 16);
    lv_obj_set_style_radius(art, 6, 0);
    lv_obj_set_style_bg_opa(art, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(art, lv_color_hex(0x2b2f36), 0);
    lv_obj_set_style_bg_grad_color(art, lv_color_hex(0x0d0f12), 0);
    lv_obj_set_style_bg_grad_dir(art, LV_GRAD_DIR_VER, 0);
    lv_obj_t *note = lv_label_create(art);
    lv_label_set_text(note, LV_SYMBOL_AUDIO);
    lv_obj_set_style_text_color(note, lv_color_hex(0x9aa2ac), 0);
    lv_obj_set_style_text_font(note, &lv_font_montserrat_22, 0);
    lv_obj_center(note);

    lv_obj_t *title = lv_label_create(S.np_screen);
    lv_label_set_text(title, "Such Great Heights");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(title, COL_TEXT, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, STATUSBAR_H + 178);

    lv_obj_t *artist = lv_label_create(S.np_screen);
    lv_label_set_text(artist, "The Postal Service");
    lv_obj_set_style_text_font(artist, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(artist, lv_color_hex(0x6b727b), 0);
    lv_obj_align(artist, LV_ALIGN_TOP_MID, 0, STATUSBAR_H + 200);

    S.np_bar = lv_bar_create(S.np_screen);
    lv_obj_set_size(S.np_bar, UI_W - 40, 6);
    lv_obj_align(S.np_bar, LV_ALIGN_TOP_MID, 0, STATUSBAR_H + 236);
    lv_obj_set_style_bg_color(S.np_bar, COL_BAR_BOT, 0);
    lv_obj_set_style_bg_color(S.np_bar, COL_ACCENT, LV_PART_INDICATOR);
    lv_obj_set_style_radius(S.np_bar, 3, 0);
    lv_obj_set_style_radius(S.np_bar, 3, LV_PART_INDICATOR);
    lv_bar_set_range(S.np_bar, 0, 1000);
    lv_bar_set_value(S.np_bar, 0, LV_ANIM_OFF);

    S.np_elapsed = lv_label_create(S.np_screen);
    lv_label_set_text(S.np_elapsed, "0:00");
    lv_obj_set_style_text_font(S.np_elapsed, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(S.np_elapsed, lv_color_hex(0x6b727b), 0);
    lv_obj_align(S.np_elapsed, LV_ALIGN_TOP_LEFT, 20, STATUSBAR_H + 246);

    S.np_remain = lv_label_create(S.np_screen);
    lv_label_set_text(S.np_remain, "-3:24");
    lv_obj_set_style_text_font(S.np_remain, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(S.np_remain, lv_color_hex(0x6b727b), 0);
    lv_obj_align(S.np_remain, LV_ALIGN_TOP_RIGHT, -20, STATUSBAR_H + 246);
}

/* Track length in seconds for the fake progress readout. */
#define NP_TRACK_SECONDS 204

static void np_progress_cb(void *var, int32_t v)
{
    (void)var;
    lv_bar_set_value(S.np_bar, v, LV_ANIM_OFF);

    int elapsed = (int)((int64_t)v * NP_TRACK_SECONDS / 1000);
    int remain  = NP_TRACK_SECONDS - elapsed;
    char buf[16];
    snprintf(buf, sizeof(buf), "%d:%02d", elapsed / 60, elapsed % 60);
    lv_label_set_text(S.np_elapsed, buf);
    snprintf(buf, sizeof(buf), "-%d:%02d", remain / 60, remain % 60);
    lv_label_set_text(S.np_remain, buf);
}

static void now_playing_start_anim(void)
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, S.np_bar);
    lv_anim_set_values(&a, 0, 1000);
    lv_anim_set_time(&a, 6000);
    lv_anim_set_exec_cb(&a, np_progress_cb);
    lv_anim_start(&a);
}

/* ---- navigation ---------------------------------------------------------- */

static void show_screen(screen_id_t id)
{
    S.current = id;
    switch (id) {
        case SCREEN_MENU:        lv_screen_load(S.menu.screen);  break;
        case SCREEN_SONGS:       lv_screen_load(S.songs.screen); break;
        case SCREEN_NOWPLAYING:
            lv_screen_load(S.np_screen);
            now_playing_start_anim();
            break;
    }
}

void ui_send_key(ui_key_t key)
{
    switch (S.current) {
        case SCREEN_MENU:
            if (key == UI_KEY_NEXT) list_apply_selection(&S.menu, S.menu.sel + 1);
            else if (key == UI_KEY_PREV) list_apply_selection(&S.menu, S.menu.sel - 1);
            else if (key == UI_KEY_SELECT) {
                if (S.menu.sel == 0) show_screen(SCREEN_SONGS);       /* Music */
                else show_screen(SCREEN_NOWPLAYING);                  /* stub  */
            }
            break;

        case SCREEN_SONGS:
            if (key == UI_KEY_NEXT) list_apply_selection(&S.songs, S.songs.sel + 1);
            else if (key == UI_KEY_PREV) list_apply_selection(&S.songs, S.songs.sel - 1);
            else if (key == UI_KEY_SELECT) show_screen(SCREEN_NOWPLAYING);
            else if (key == UI_KEY_MENU) show_screen(SCREEN_MENU);
            break;

        case SCREEN_NOWPLAYING:
            if (key == UI_KEY_MENU) show_screen(SCREEN_SONGS);
            break;
    }
}

void ui_init(void)
{
    build_list_view(&S.menu,  "iPod",  MENU_ITEMS, (int)(sizeof(MENU_ITEMS) / sizeof(MENU_ITEMS[0])), true);
    build_list_view(&S.songs, "Songs", SONG_ITEMS, (int)(sizeof(SONG_ITEMS) / sizeof(SONG_ITEMS[0])), false);
    build_now_playing();
    show_screen(SCREEN_MENU);
}
