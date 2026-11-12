#ifndef SCORE_UI_H
#define SCORE_UI_H

#include "game_state.h"
#include "ui_screens.h"

enum { SCORE_DISPLAY_SIZE = 240 };

typedef struct {
    GameState *game;
    const UiScreens *screens;
    lv_obj_t *player_label;
    lv_obj_t *score_label;
    lv_obj_t *added_label;
    lv_obj_t *ring;
    lv_indev_t *hold_input;
    lv_point_t press_origin;
    int drag_start_score;
    bool drag_scoring;
    int minus_hold_delay;
    int plus_hold_delay;
    bool holding_minus_icon;
    bool holding_plus_icon;
} ScoreUi;

void score_ui_create(ScoreUi *ui, const UiScreens *screens);

#endif
