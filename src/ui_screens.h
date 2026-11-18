#ifndef UI_SCREENS_H
#define UI_SCREENS_H

#include "lvgl.h"

enum { DISPLAY_SIZE = 240 };


typedef struct {
    lv_obj_t *home;
    lv_obj_t *game_select;
    lv_obj_t *add_player;
    lv_obj_t *score;
    lv_obj_t *welcome;
    void (*open_screen)(lv_obj_t *destination, uint32_t dissolve_ms);
} UiScreens;

#endif
