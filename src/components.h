#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "ui_screens.h"

typedef struct {
    const UiScreens *screens;
    lv_obj_t *ring;
    lv_indev_t *input;
} HomeHold;

void home_hold_create(HomeHold *hold, lv_obj_t *screen, const UiScreens *screens);
void home_hold_start(HomeHold *hold, lv_indev_t *input);
void home_hold_cancel(HomeHold *hold);
void create_round_screen_background(lv_obj_t *screen);

lv_obj_t *create_app_icon(lv_obj_t *parent, const lv_image_dsc_t *source,
                         lv_event_cb_t callback, void *data);

lv_obj_t *create_icon_button_with_color(lv_obj_t *parent, const char *icon,
                             lv_event_cb_t callback, void *data, lv_color_t color);


lv_obj_t *create_icon_button(lv_obj_t *parent, const char *icon,
                             lv_event_cb_t callback, void *data);

lv_obj_t *create_button_with_color(lv_obj_t *parent, const char *text,
                             lv_event_cb_t callback, void *data, lv_color_t color);


lv_obj_t *create_button(lv_obj_t *parent, const char *text,
                             lv_event_cb_t callback, void *data);

lv_obj_t *create_label(lv_obj_t *parent, const char *text);

#endif
