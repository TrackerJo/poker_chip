#ifndef SCORE_UI_H
#define SCORE_UI_H

#include "game_state.h"
#include "ui_screens.h"
#include "components.h"


typedef struct {
    GameState *game;
    const UiScreens *screens;
    lv_obj_t *player_label;
    lv_obj_t *score_label;
    lv_obj_t *added_label;
    lv_obj_t *minus_icon;
    lv_obj_t *plus_icon;
    lv_obj_t *leaderboard_icon;
    lv_obj_t *ring;
    lv_indev_t *hold_input;
    lv_point_t press_origin;
    int drag_start_score;
    bool drag_scoring;
    int minus_hold_delay;
    int plus_hold_delay;
    bool holding_minus_icon;
    bool holding_plus_icon;
    lv_obj_t *score_view;
    lv_obj_t *leaderboard_view;
    lv_obj_t *leaderboard_list;
} ScoreUi;

void score_ui_create(ScoreUi *ui, const UiScreens *screens);

#endif
