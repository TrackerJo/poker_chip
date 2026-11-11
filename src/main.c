#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

#include "score_ui.h"
#include "game_state.h"

int main(void)
{

    GameState game = {
        .kind = GAME_POINT_COUNTER,
        .player_count = 2,
        .active_player = 0,
    };
    
    SDL_SetMainReady();
    lv_init();

    lv_display_t *display = lv_sdl_window_create(SCORE_DISPLAY_SIZE, SCORE_DISPLAY_SIZE);
    if (display == NULL) {
        fprintf(stderr, "Unable to open the poker chip display.\n");
        return EXIT_FAILURE;
    }

    lv_sdl_window_set_title(display, "Poker Chip");
    lv_sdl_mouse_create();
    score_ui_create(lv_display_get_screen_active(display), &game);

    while (lv_display_get_default() != NULL) {
        uint32_t delay_ms = lv_timer_handler();
        SDL_Delay(delay_ms == LV_NO_TIMER_READY ? LV_DEF_REFR_PERIOD : delay_ms);
    }

    return EXIT_SUCCESS;
}
