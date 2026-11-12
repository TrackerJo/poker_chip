#include "game_state.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>

void game_start(GameState *game, const GameSettings *settings)
{
    *game = (GameState){ .settings = settings };
}

int game_min_score(const GameState *game)
{
    return game->settings && game->settings->allow_negative_scores ? INT_MIN : 0;
}

bool game_add_point(GameState *game, size_t player_index)
{
    if (player_index >= game->player_count || player_index >= MAX_PLAYERS)
    {
        return false;
    }

    Player *player = &game->players[player_index];
    if (player->score == INT_MAX)
    {
        return false;
    }

    player->score++;
    return true;
}

bool game_subtract_point(GameState *game, size_t player_index)
{
    if (player_index >= game->player_count || player_index >= MAX_PLAYERS)
    {
        return false;
    }

    Player *player = &game->players[player_index];
    if (player->score == INT_MAX)
    {
        return false;
    }
    if (player->score <= game_min_score(game)) {
        return false;
    }
    player->score--;
    return true;
}

bool game_set_score(GameState *game, size_t player_index, int score)
{
    if (player_index >= game->player_count || player_index >= MAX_PLAYERS ||
        score < game_min_score(game))
    {
        return false;
    }

    Player *player = &game->players[player_index];
    if (player->score == score)
    {
        return false;
    }

    player->score = score;
    return true;
}

size_t game_rotate_active_player_right(GameState *game)
{
    if (game->player_count == 0 || game->active_player + 1 == game->player_count)
    {
        game->active_player = 0;
        return 0;
    }

    game->active_player += 1;
    return game->active_player;
}

size_t game_rotate_active_player_left(GameState *game)
{
    if (game->player_count == 0)
    {
        return 0;
    }

    if (game->active_player == 0)
    {
        game->active_player = game->player_count - 1;
        return game->player_count - 1;
    }

    game->active_player -= 1;
    return game->active_player;
}

bool game_add_player(GameState *game, const char *name, size_t score)
{
    if (game->player_count >= MAX_PLAYERS || score > INT_MAX ||
        strlen(name) >= MAX_PLAYER_NAME_LENGTH ||
        name[strspn(name, " \t\r\n")] == '\0')
    {
        return false;
    }

    Player *player = &game->players[game->player_count];
    *player = (Player){
        .id = game->total_player_count,
        .score = score,
    };
    snprintf(player->name, sizeof player->name, "%s", name);
    game->player_count += 1;
    game->total_player_count += 1;
    return true;
}

void game_remove_player(GameState *game, size_t id)
{
    size_t index = 0;
    while (index < game->player_count && game->players[index].id != id)
    {
        index++;
    }
    if (index == game->player_count)
    {
        return;
    }

    for (size_t i = index + 1; i < game->player_count; i++)
    {
        game->players[i - 1] = game->players[i];
    }

    game->player_count--;
    game->players[game->player_count] = (Player){0};

    if (game->player_count == 0)
    {
        game->active_player = 0;
    }
    else if (index < game->active_player || game->active_player == game->player_count)
    {
        game->active_player--;
    }
}
