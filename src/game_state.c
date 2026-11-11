#include "game_state.h"
#include <limits.h>

bool game_add_point(GameState *game, size_t player_index)
{
    if (player_index >= game->player_count || player_index >= MAX_PLAYERS) {
        return false;
    }

    Player *player = &game->players[player_index];
    if (player->score == INT_MAX) {
        return false;
    }

    player->score++;
    return true;
}
