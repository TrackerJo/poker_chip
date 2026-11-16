#include "welcome_ui.h"
#include <stdlib.h>
#include <string.h>

LV_IMAGE_DECLARE(mountains);

enum {
    WELCOME_DISSOLVE_MS = 420,
    DISSOLVE_TILE_SIZE = 4,
    DISSOLVE_STEPS = 256,
    DISSOLVE_COLOR_STRIDE = LV_DRAW_BUF_STRIDE(DISPLAY_SIZE, LV_COLOR_FORMAT_RGB565),
    DISSOLVE_ALPHA_STRIDE = DISSOLVE_COLOR_STRIDE / 2,
    DISSOLVE_IMAGE_BYTES = (DISSOLVE_COLOR_STRIDE + DISSOLVE_ALPHA_STRIDE) * DISPLAY_SIZE,
};

typedef struct {
    lv_image_dsc_t image;
    uint8_t *alpha;
    uint8_t pixels[DISSOLVE_IMAGE_BYTES + LV_DRAW_BUF_ALIGN];
} WelcomeDissolve;

static void dissolve_tiles(void *object, int32_t progress)
{
    lv_obj_t *overlay = object;
    WelcomeDissolve *dissolve = lv_obj_get_user_data(overlay);

    uint32_t tile = 0;
    for (int32_t y = 0; y < DISPLAY_SIZE; y += DISSOLVE_TILE_SIZE) {
        for (int32_t x = 0; x < DISPLAY_SIZE; x += DISSOLVE_TILE_SIZE) {
            /* Give each tile a fixed reveal threshold so revealed tiles stay clear. */
            uint32_t hash = tile++ * 0x9E3779B9u;
            uint8_t threshold = (uint8_t)(hash ^ (hash >> 16));
            uint8_t opacity = threshold < progress ? LV_OPA_TRANSP : LV_OPA_COVER;
            for (int32_t row = y; row < y + DISSOLVE_TILE_SIZE; row++) {
                memset(dissolve->alpha + row * DISSOLVE_ALPHA_STRIDE + x, opacity,
                       DISSOLVE_TILE_SIZE);
            }
        }
    }

    lv_obj_invalidate(overlay);
}

static void free_dissolve(lv_event_t *event)
{
    free(lv_event_get_user_data(event));
}

bool welcome_ui_dissolve_to(lv_obj_t *destination, uint32_t duration_ms)
{
    lv_display_t *display = lv_obj_get_display(destination);
    lv_obj_t *welcome = lv_display_get_screen_active(display);
    if (welcome == destination) return false;

    /* The temporary snapshot is larger than the simulator's LVGL heap. */
    WelcomeDissolve *dissolve = malloc(sizeof *dissolve);
    if (dissolve == NULL) return false;

    lv_draw_buf_t snapshot;
    if (lv_draw_buf_init(&snapshot, DISPLAY_SIZE, DISPLAY_SIZE, LV_COLOR_FORMAT_RGB565,
                         DISSOLVE_COLOR_STRIDE, dissolve->pixels, sizeof dissolve->pixels) != LV_RESULT_OK ||
        lv_snapshot_take_to_draw_buf(welcome, LV_COLOR_FORMAT_RGB565, &snapshot) != LV_RESULT_OK ||
        snapshot.header.w != DISPLAY_SIZE || snapshot.header.h != DISPLAY_SIZE ||
        snapshot.header.stride != DISSOLVE_COLOR_STRIDE) {
        free(dissolve);
        return false;
    }

    dissolve->image = (lv_image_dsc_t){
        .header.magic = LV_IMAGE_HEADER_MAGIC,
        .header.cf = LV_COLOR_FORMAT_RGB565A8,
        .header.w = DISPLAY_SIZE,
        .header.h = DISPLAY_SIZE,
        .header.stride = DISSOLVE_COLOR_STRIDE,
        .data_size = DISSOLVE_IMAGE_BYTES,
        .data = snapshot.data,
    };
    dissolve->alpha = snapshot.data + DISSOLVE_COLOR_STRIDE * DISPLAY_SIZE;
    memset(dissolve->alpha, LV_OPA_COVER, DISSOLVE_ALPHA_STRIDE * DISPLAY_SIZE);

    lv_obj_t *overlay = lv_image_create(destination);
    lv_obj_set_user_data(overlay, dissolve);
    lv_obj_add_event_cb(overlay, free_dissolve, LV_EVENT_DELETE, dissolve);
    lv_obj_set_clickable(overlay, true);
    lv_obj_set_press_lock(overlay, true);
    lv_image_set_src(overlay, &dissolve->image);
    lv_obj_center(overlay);

    lv_anim_t animation;
    lv_anim_init(&animation);
    lv_anim_set_var(&animation, overlay);
    lv_anim_set_exec_cb(&animation, dissolve_tiles);
    lv_anim_set_values(&animation, 0, DISSOLVE_STEPS);
    lv_anim_set_duration(&animation, duration_ms);
    lv_anim_set_path_cb(&animation, lv_anim_path_linear);
    lv_anim_set_completed_cb(&animation, lv_obj_delete_anim_completed_cb);
    if (lv_anim_start(&animation) == NULL) {
        lv_obj_delete(overlay);
        return false;
    }

    lv_screen_load(destination);
    return true;
}

static void open_home_screen(lv_event_t *event)
{
    const UiScreens *screens = lv_event_get_user_data(event);
    screens->open_screen(screens->home, WELCOME_DISSOLVE_MS);
}

void welcome_ui_create(UiScreens *screens)
{
    lv_obj_t *screen = screens->welcome;
    lv_obj_remove_style_all(screen);
    lv_obj_set_scrollable(screen, false);
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    lv_obj_t *face = lv_obj_create(screen);
    lv_obj_remove_style_all(face);
    lv_obj_set_scrollable(face, false);
    lv_obj_set_clickable(face, true);
    lv_obj_set_press_lock(face, true);
    lv_obj_set_gesture_bubble(face, false);
    lv_obj_set_size(face, DISPLAY_SIZE, DISPLAY_SIZE);
    lv_obj_center(face);
    lv_obj_set_style_radius(face, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(face, lv_color_hex(0x729adb), 0);
    lv_obj_set_style_bg_opa(face, LV_OPA_COVER, 0);
    lv_obj_add_event_cb(face, open_home_screen, LV_EVENT_CLICKED,screens);

    lv_obj_t *image = lv_image_create(face);
    lv_image_set_src(image, &mountains);
    lv_obj_center(image);

    // lv_obj_t *ring = lv_obj_create(face);
    // lv_obj_remove_style_all(ring);
    // lv_obj_set_scrollable(ring, false);
    // lv_obj_set_clickable(ring, false);
    // lv_obj_set_size(ring, 240, 240);
    // lv_obj_center(ring);
    // lv_obj_set_style_radius(ring, LV_RADIUS_CIRCLE, 0);
    // lv_obj_set_style_border_width(ring, 10, 0);
    // lv_obj_set_style_border_color(ring, lv_color_hex(0x5084df), 0);

    lv_obj_t *hello = lv_label_create(face);
    lv_obj_set_width(hello, 180);
    lv_label_set_long_mode(hello, LV_LABEL_LONG_MODE_DOTS);
    lv_obj_set_style_text_align(hello, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(hello, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(hello, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_letter_space(hello, 2, 0);
    lv_label_set_text(hello, "Hello");
    lv_obj_align(hello, LV_ALIGN_TOP_MID, 0, 18);

    lv_obj_t *name = lv_label_create(face);
    lv_obj_set_width(name, 180);
    lv_label_set_long_mode(name, LV_LABEL_LONG_MODE_DOTS);
    lv_obj_set_style_text_align(name, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(name, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(name, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_letter_space(name, 2, 0);
    lv_label_set_text(name, "Nathaniel");
    lv_obj_align(name, LV_ALIGN_BOTTOM_MID, 0, -18);
}
