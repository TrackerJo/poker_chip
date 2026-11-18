#ifndef GAME_SETTINGS_H
#define GAME_SETTINGS_H

#include <stdbool.h>
#include <stddef.h>

enum { MAX_SCORE_FIELDS = 3 };

typedef enum {
    GAME_POINT_COUNTER,
    EUCHRE,
    OH_HELL,
    PINOCHLE
} GameKind;

enum { PINOCHLE_BID, PINOCHLE_POINTS, PINOCHLE_WINNER, PINOCHLE_FIELD_COUNT };

typedef struct {
    const char *label;
    int min_value;
    int max_value;
    bool hidden;
} ScoreField;

typedef struct {
    GameKind kind;
    bool allow_negative_scores;
    int max_players;
    size_t score_field_count;
    ScoreField score_fields[MAX_SCORE_FIELDS];
    int (*score_hand)(const int values[MAX_SCORE_FIELDS]);
} GameSettings;

#endif
