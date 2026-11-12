#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <stdbool.h>
#include <stddef.h>

#include "game_settings.h"

enum { MAX_PLAYERS = 8, MAX_PLAYER_NAME_LENGTH = 64 };

typedef struct {
    size_t id;
    int score;
    char name[MAX_PLAYER_NAME_LENGTH];
} Player;

typedef struct {
    const GameSettings *settings;
    Player players[MAX_PLAYERS];
    size_t player_count;
    size_t total_player_count;
    size_t active_player;
} GameState;

void game_start(GameState *game, const GameSettings *settings);
int game_min_score(const GameState *game);

bool game_add_point(GameState *game, size_t player_index);
bool game_subtract_point(GameState *game, size_t player_index);
bool game_set_score(GameState *game, size_t player_index, int score);

bool game_add_player(GameState *game, const char *name, size_t score);
void game_remove_player(GameState *game, size_t id);

size_t game_rotate_active_player_right(GameState *game);
size_t game_rotate_active_player_left(GameState *game);

#endif
