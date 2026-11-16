#include "game_state.h"
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool game_start(GameState *game, const GameSettings *settings)
{
    if (settings == NULL || settings->max_players <= 0 ||
        settings->score_field_count == 0 || settings->score_field_count > MAX_SCORE_FIELDS ||
        (settings->score_field_count > 1 && settings->score_hand == NULL)) {
        return false;
    }

    Player *players = calloc((size_t)settings->max_players, sizeof *players);
    if (players == NULL) {
        return false;
    }

    game_destroy(game);
    *game = (GameState){
        .settings = settings,
        .players = players,
    };
    return true;
}

void game_destroy(GameState *game)
{
    free(game->players);
    while (game->last_hand != NULL) {
        GameHand *previous = game->last_hand->previous;
        free(game->last_hand);
        game->last_hand = previous;
    }
    *game = (GameState){0};
}

int game_min_score(const GameState *game)
{
    return game->settings && game->settings->allow_negative_scores ? INT_MIN : 0;
}

bool game_add_point(GameState *game, size_t player_index)
{
    if (player_index >= game->player_count || player_index >= game->settings->max_players)
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
    if (player_index >= game->player_count || player_index >= game->settings->max_players)
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
    if (player_index >= game->player_count || player_index >= game->settings->max_players ||
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

bool game_get_field(const GameState *game, size_t player, size_t field, int *value)
{
    if (player >= game->player_count || field >= game->settings->score_field_count)
        return false;

    const Player *entry = &game->players[player];
    if (game->settings->score_field_count == 1) {
        *value = entry->score;
        return true;
    }

    if (!entry->hand_values[field].entered)
        return false;

    *value = entry->hand_values[field].value;
    return true;
}

bool game_set_field(GameState *game, size_t player, size_t field, int value)
{
    if (player >= game->player_count || field >= game->settings->score_field_count)
        return false;

    const ScoreField *definition = &game->settings->score_fields[field];
    if (value < definition->min_value || value > definition->max_value)
        return false;

    if (game->settings->score_field_count == 1)
        return game_set_score(game, player, value);

    HandValue *input = &game->players[player].hand_values[field];
    if (input->entered && input->value == value)
        return false;

    *input = (HandValue){ .value = value, .entered = true };
    return true;
}

static bool read_hand_entry(const GameState *game, size_t player, HandEntry *entry,
                            size_t *missing_field)
{
    for (size_t field = 0; field < game->settings->score_field_count; field++) {
        if (!game_get_field(game, player, field, &entry->values[field])) {
            *missing_field = field;
            return false;
        }
    }

    entry->player_id = game->players[player].id;
    entry->points_earned = game->settings->score_hand(entry->values);
    return true;
}

bool game_preview_hand_score(const GameState *game, size_t player, int *points)
{
    if (game->settings == NULL || game->settings->score_field_count <= 1 ||
        player >= game->player_count)
        return false;

    HandEntry entry = {0};
    size_t missing_field;
    if (!read_hand_entry(game, player, &entry, &missing_field))
        return false;

    *points = entry.points_earned;
    return true;
}

FinishHandResult game_finish_hand(GameState *game, size_t *missing_player,
                                 size_t *missing_field)
{
    if (game->settings == NULL || game->settings->score_field_count <= 1 ||
        game->player_count == 0)
        return FINISH_HAND_UNAVAILABLE;

    if (game->player_count > (SIZE_MAX - sizeof(GameHand)) / sizeof(HandEntry))
        return FINISH_HAND_NO_MEMORY;

    GameHand *hand = calloc(1, sizeof *hand + game->player_count * sizeof(HandEntry));
    if (hand == NULL)
        return FINISH_HAND_NO_MEMORY;

    for (size_t player = 0; player < game->player_count; player++) {
        HandEntry *entry = &hand->entries[player];
        if (!read_hand_entry(game, player, entry, missing_field)) {
            *missing_player = player;
            free(hand);
            return FINISH_HAND_MISSING_FIELD;
        }

        int64_t total = (int64_t)game->players[player].score + entry->points_earned;
        if (total < game_min_score(game) || total > INT_MAX) {
            free(hand);
            return FINISH_HAND_SCORE_OUT_OF_RANGE;
        }
    }

    for (size_t player = 0; player < game->player_count; player++) {
        game->players[player].score += hand->entries[player].points_earned;
        memset(game->players[player].hand_values, 0, sizeof game->players[player].hand_values);
    }

    hand->entry_count = game->player_count;
    hand->previous = game->last_hand;
    game->last_hand = hand;
    return FINISH_HAND_OK;
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
    if (game->player_count >= game->settings->max_players || score > INT_MAX ||
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
