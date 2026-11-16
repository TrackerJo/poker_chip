#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "lvgl.h"

lv_obj_t *create_icon_button_with_color(lv_obj_t *parent, const char *icon,
                             lv_event_cb_t callback, void *data, lv_color_t color);


lv_obj_t *create_icon_button(lv_obj_t *parent, const char *icon,
                             lv_event_cb_t callback, void *data);

lv_obj_t *create_label(lv_obj_t *parent, const char *text);

#endif
