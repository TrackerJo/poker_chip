#include "home_ui.h"

static void open_add_player(lv_event_t *event)
{
    const UiScreens *screens = lv_event_get_user_data(event);
    screens->open_screen(screens->add_player);
}

void home_ui_create(UiScreens *screens)
{
    lv_obj_t *screen = screens->home;
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

    lv_obj_t *ring = lv_obj_create(face);
    lv_obj_remove_style_all(ring);
    lv_obj_set_scrollable(ring, false);
    lv_obj_set_clickable(ring, false);
    lv_obj_set_size(ring, 240, 240);
    lv_obj_center(ring);
    lv_obj_set_style_radius(ring, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(ring, 10, 0);
    lv_obj_set_style_border_color(ring, lv_color_hex(0x5084df), 0);

    lv_obj_t *game_button = lv_button_create(screen);
    lv_obj_center(game_button);

    lv_obj_t *game_label = lv_label_create(game_button);
    lv_label_set_text(game_label, "Point Counter");
    lv_obj_center(game_label);
    lv_obj_add_event_cb(game_button, open_add_player, LV_EVENT_CLICKED, screens);
}
