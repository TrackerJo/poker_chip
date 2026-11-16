#ifndef ADD_PLAYER_UI_H
#define ADD_PLAYER_UI_H

#include "game_state.h"
#include "ui_screens.h"
#include "components.h"

typedef struct {
    GameState *game;
    const UiScreens *screens;
    lv_obj_t *name_input;
    lv_obj_t *status;
    lv_obj_t *name_view;
    lv_obj_t *roster_view;
    lv_obj_t *roster_list;
    lv_obj_t *add_icon;
    lv_obj_t *edit_icon;
    lv_obj_t *play_icon;
    HomeHold home_hold;
} AddPlayerUi;

void add_player_ui_create(AddPlayerUi *ui, const UiScreens *screens);

#endif
