#ifndef HOME_UI_H
#define HOME_UI_H

#include "ui_screens.h"
#include "game_state.h"
#include "components.h"
#include "score_ui.h"

typedef struct {
    GameState *game;
    const GameSettings *settings;
    UiScreens *screens;
    ScoreUi *score_ui;
} HomeGameContext;

void home_ui_create(UiScreens *screens, GameState *game, ScoreUi *score_ui);

#endif
