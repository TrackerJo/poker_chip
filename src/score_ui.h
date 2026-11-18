#ifndef SCORE_UI_H
#define SCORE_UI_H

#include "game_state.h"
#include "ui_screens.h"
#include "components.h"


typedef struct {
    GameState *game;
    const UiScreens *screens;
    size_t active_field;
    lv_obj_t *field_tabs;
    lv_obj_t *player_label;
    lv_obj_t *score_label;
    lv_obj_t *hand_status;
    lv_timer_t *hand_status_timer;
    lv_obj_t *finish_hand_button;
    lv_obj_t *added_label;
    lv_obj_t *minus_icon;
    lv_obj_t *plus_icon;
    lv_obj_t *leaderboard_icon;
    HomeHold home_hold;
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
    lv_obj_t *trump_view;
    lv_obj_t *action_row;
    lv_obj_t *trump_icon;
} ScoreUi;

void score_ui_create(ScoreUi *ui, const UiScreens *screens);
void score_ui_load(ScoreUi *ui);

#endif
