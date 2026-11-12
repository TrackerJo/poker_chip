#ifndef ADD_PLAYER_UI_H
#define ADD_PLAYER_UI_H

#include "game_state.h"
#include "ui_screens.h"

typedef struct {
    GameState *game;
    const UiScreens *screens;
    lv_obj_t *name_input;
    lv_obj_t *status;
    lv_obj_t *name_view;
    lv_obj_t *roster_view;
    lv_obj_t *roster_list;
} AddPlayerUi;

void add_player_ui_create(AddPlayerUi *ui, const UiScreens *screens);

#endif
