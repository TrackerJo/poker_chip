#include "components.h"

enum { HOME_RING_WIDTH = 10, HOME_FILL_TIME_MS = 1000 };

static void set_ring_width(void *ring, int32_t width)
{
    lv_obj_set_style_border_width(ring, width, 0);
}

static void home_hold_completed(lv_anim_t *animation)
{
    HomeHold *hold = lv_anim_get_user_data(animation);
    lv_indev_t *input = hold->input;
    hold->input = NULL;
    lv_indev_wait_release(input);

    /* Render the final frame without running the animation timer again. */
    lv_timer_t *refresh = lv_display_get_refr_timer(lv_obj_get_display(hold->ring));
    if (refresh != NULL)
        lv_display_refr_timer(refresh);

    hold->screens->open_screen(hold->screens->home, 0);
}

void home_hold_create(HomeHold *hold, lv_obj_t *screen, const UiScreens *screens)
{
    lv_obj_t *ring = lv_obj_create(screen);
    lv_obj_remove_style_all(ring);
    lv_obj_set_scrollable(ring, false);
    lv_obj_set_clickable(ring, false);
    lv_obj_set_size(ring, DISPLAY_SIZE, DISPLAY_SIZE);
    lv_obj_center(ring);
    lv_obj_set_style_radius(ring, LV_RADIUS_CIRCLE, 0);
    set_ring_width(ring, HOME_RING_WIDTH);
    lv_obj_set_style_border_color(ring, lv_color_hex(0x5084DF), 0);
    *hold = (HomeHold){ .screens = screens, .ring = ring };
}

void home_hold_start(HomeHold *hold, lv_indev_t *input)
{
    if (input == NULL || hold->input != NULL)
        return;

    lv_anim_t fill;
    lv_anim_init(&fill);
    lv_anim_set_var(&fill, hold->ring);
    lv_anim_set_exec_cb(&fill, set_ring_width);
    lv_anim_set_values(&fill, HOME_RING_WIDTH, DISPLAY_SIZE / 2);
    lv_anim_set_duration(&fill, HOME_FILL_TIME_MS);
    lv_anim_set_path_cb(&fill, lv_anim_path_linear);
    lv_anim_set_user_data(&fill, hold);
    lv_anim_set_completed_cb(&fill, home_hold_completed);
    if (lv_anim_start(&fill) != NULL)
        hold->input = input;
}

void home_hold_cancel(HomeHold *hold)
{
    if (hold->input != NULL) {
        lv_anim_delete(hold->ring, set_ring_width);
        hold->input = NULL;
    }
    set_ring_width(hold->ring, HOME_RING_WIDTH);
}

lv_obj_t *create_icon_button_with_color(lv_obj_t *parent, const char *icon,
                             lv_event_cb_t callback, void *data, lv_color_t color)
{
    lv_obj_t *button = lv_button_create(parent);
    lv_obj_remove_style_all(button);
    lv_obj_add_event_cb(button, callback, LV_EVENT_CLICKED, data);

    lv_obj_t *label = lv_label_create(button);
    lv_label_set_text(label, icon);
    lv_obj_set_style_text_color(label, color, 0);
    lv_obj_center(label);
    return button;
}

lv_obj_t *create_icon_button(lv_obj_t *parent, const char *icon,
                             lv_event_cb_t callback, void *data)
{
    return create_icon_button_with_color(parent, icon, callback, data,
                                         lv_color_white());
}

lv_obj_t *create_button_with_color(lv_obj_t *parent, const char *text,
                             lv_event_cb_t callback, void *data, lv_color_t color)
{
    lv_obj_t *button = lv_button_create(parent);
    lv_obj_add_event_cb(button, callback, LV_EVENT_CLICKED, data);

    lv_obj_t *label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, color, 0);
    lv_obj_center(label);
    return button;
}

lv_obj_t *create_button(lv_obj_t *parent, const char *text,
                             lv_event_cb_t callback, void *data)
{
    return create_button_with_color(parent, text, callback, data,
                                         lv_color_white());
}

lv_obj_t *create_label(lv_obj_t *parent, const char *text)
{
    lv_obj_t *label = lv_label_create(parent);
    if (text != NULL)
    {
        lv_label_set_text(label, text);
    }
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_letter_space(label, 2, 0);
    return label;
}
