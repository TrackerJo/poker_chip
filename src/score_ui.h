#ifndef SCORE_UI_H
#define SCORE_UI_H

#include "lvgl.h"
#include "game_state.h"

enum { SCORE_DISPLAY_SIZE = 240 };

void score_ui_create(lv_obj_t *screen, GameState *game);
void score_text_clicked(lv_event_t * e);

#endif
