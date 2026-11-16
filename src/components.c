#include "components.h"

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
