#ifndef GAME_SETTINGS_H
#define GAME_SETTINGS_H

#include <stdbool.h>

typedef enum {
    GAME_POINT_COUNTER,
} GameKind;

typedef struct {
    GameKind kind;
    bool allow_negative_scores;
} GameSettings;

#endif
