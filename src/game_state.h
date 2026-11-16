#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <stdbool.h>
#include <stddef.h>

#include "game_settings.h"

enum { MAX_PLAYERS = 8, MAX_PLAYER_NAME_LENGTH = 64 };

typedef struct {
    int value;
    bool entered;
} HandValue;

typedef struct {
    size_t id;
    int score;
    char name[MAX_PLAYER_NAME_LENGTH];
    HandValue hand_values[MAX_SCORE_FIELDS];
} Player;

typedef struct {
    size_t player_id;
    int values[MAX_SCORE_FIELDS];
    int points_earned;
} HandEntry;

typedef struct GameHand {
    struct GameHand *previous;
    size_t entry_count;
    HandEntry entries[];
} GameHand;

typedef struct {
    const GameSettings *settings;
    Player *players;
    size_t player_count;
    size_t total_player_count;
    size_t active_player;
    GameHand *last_hand;
} GameState;

typedef enum {
    FINISH_HAND_OK,
    FINISH_HAND_MISSING_FIELD,
    FINISH_HAND_SCORE_OUT_OF_RANGE,
    FINISH_HAND_NO_MEMORY,
    FINISH_HAND_UNAVAILABLE
} FinishHandResult;

bool game_start(GameState *game, const GameSettings *settings);
void game_destroy(GameState *game);
int game_min_score(const GameState *game);

bool game_add_point(GameState *game, size_t player_index);
bool game_subtract_point(GameState *game, size_t player_index);
bool game_set_score(GameState *game, size_t player_index, int score);
bool game_get_field(const GameState *game, size_t player, size_t field, int *value);
bool game_set_field(GameState *game, size_t player, size_t field, int value);
bool game_preview_hand_score(const GameState *game, size_t player, int *points);
FinishHandResult game_finish_hand(GameState *game, size_t *missing_player,
                                 size_t *missing_field);

bool game_add_player(GameState *game, const char *name, size_t score);
void game_remove_player(GameState *game, size_t id);

size_t game_rotate_active_player_right(GameState *game);
size_t game_rotate_active_player_left(GameState *game);

#endif
