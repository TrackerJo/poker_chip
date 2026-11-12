#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

#include "score_ui.h"
#include "game_state.h"
#include "home_ui.h"
#include "add_player_ui.h"

static const GameSettings point_counter_settings = {
    .kind = GAME_POINT_COUNTER,
    .allow_negative_scores = true,
};

int main(void)
{
    GameState game;
    game_start(&game, &point_counter_settings);
    ScoreUi score_ui = { .game = &game };

    SDL_SetMainReady();
    lv_init();

    lv_display_t *display = lv_sdl_window_create(SCORE_DISPLAY_SIZE, SCORE_DISPLAY_SIZE);
    if (display == NULL) {
        fprintf(stderr, "Unable to open the poker chip display.\n");
        return EXIT_FAILURE;
    }

    UiScreens screens = {
        .home = lv_display_get_screen_active(display),
        .add_player = lv_obj_create(NULL),
        .score = lv_obj_create(NULL),
        .open_screen = lv_screen_load,
    };
    score_ui_create(&score_ui, &screens);
    AddPlayerUi add_player_ui = { .game = &game };
    add_player_ui_create(&add_player_ui, &screens);
    home_ui_create(&screens);

    lv_sdl_window_set_title(display, "Poker Chip");
    lv_sdl_mouse_create();

    while (lv_display_get_default() != NULL) {
        uint32_t delay_ms = lv_timer_handler();
        SDL_Delay(delay_ms == LV_NO_TIMER_READY ? LV_DEF_REFR_PERIOD : delay_ms);
    }

    return EXIT_SUCCESS;
}
