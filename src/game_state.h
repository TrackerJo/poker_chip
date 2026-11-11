#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <stdbool.h>
#include <stddef.h>

enum { MAX_PLAYERS = 8 };

typedef enum {
    GAME_POINT_COUNTER,
} GameKind;

typedef struct {
    int score;
} Player;

typedef struct {
    GameKind kind;
    Player players[MAX_PLAYERS];
    size_t player_count;
    size_t active_player;
} GameState;

bool game_add_point(GameState *game, size_t player_index);

#endif
