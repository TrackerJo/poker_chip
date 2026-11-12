#ifndef UI_SCREENS_H
#define UI_SCREENS_H

#include "lvgl.h"

typedef struct {
    lv_obj_t *home;
    lv_obj_t *add_player;
    lv_obj_t *score;
    void (*open_screen)(lv_obj_t *destination);
} UiScreens;

#endif
