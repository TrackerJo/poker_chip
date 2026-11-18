#include "game_select_ui.h"
#include "components.h"
#include <limits.h>

static const GameSettings point_counter_settings = {
    .kind = GAME_POINT_COUNTER,
    .allow_negative_scores = true,
    .max_players = 8,
    .score_field_count = 1,
    .score_fields = {{ .label = "Score", .min_value = INT_MIN, .max_value = INT_MAX }}
};

static const GameSettings euchre_settings = {
    .kind = EUCHRE,
    .allow_negative_scores = false,
    .max_players = 2,
    .score_field_count = 1,
    .score_fields = {{ .label = "Score", .min_value = 0, .max_value = INT_MAX }}
};

enum { OH_HELL_BID, OH_HELL_TRICKS, OH_HELL_FIELD_COUNT };

static int score_oh_hell_hand(const int values[MAX_SCORE_FIELDS])
{
    int bid = values[OH_HELL_BID];
    return bid == values[OH_HELL_TRICKS] ? 10 + bid : values[OH_HELL_TRICKS];
}

static const GameSettings oh_hell_settings = {
    .kind = OH_HELL,
    .max_players = 7,
    .score_field_count = OH_HELL_FIELD_COUNT,
    .score_fields = {
        [OH_HELL_BID] = { .label = "Bid", .max_value = 17 },
        [OH_HELL_TRICKS] = { .label = "Tricks", .max_value = 17 },
    },
    .score_hand = score_oh_hell_hand,
};

typedef struct {
    const UiScreens *screens;
    GameState *game;
    ScoreUi *score_ui;
    const GameSettings *settings;
} GameSelection;

static const struct {
    const char *label;
    const GameSettings *settings;
} games[] = {
    { "Euchre", &euchre_settings },
    { "Oh Hell", &oh_hell_settings },
    { "Score Counter", &point_counter_settings },
};

static void open_game(lv_event_t *event)
{
    const GameSelection *selection = lv_event_get_user_data(event);
    if (!game_start(selection->game, selection->settings)) {
        return;
    }
    score_ui_load(selection->score_ui);
    selection->screens->open_screen(selection->screens->add_player, 0);
}

static void open_home(lv_event_t *event)
{
    const UiScreens *screens = lv_event_get_user_data(event);
    screens->open_screen(screens->home, 0);
}

void game_select_ui_create(const UiScreens *screens, GameState *game, ScoreUi *score_ui)
{
    lv_obj_t *screen = screens->game_select;
    create_round_screen_background(screen);

    lv_obj_t *title = create_label(screen, "Select a game");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_letter_space(title, 0, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);

    lv_obj_t *list = lv_obj_create(screen);
    lv_obj_remove_style_all(list);
    lv_obj_set_scrollable(list, false);
    lv_obj_set_clickable(list, false);
    lv_obj_set_size(list, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(list, 6, 0);
    lv_obj_align(list, LV_ALIGN_CENTER, 0, 4);

    static GameSelection selections[sizeof games / sizeof games[0]];
    for (size_t i = 0; i < sizeof games / sizeof games[0]; i++) {
        selections[i] = (GameSelection){
            .screens = screens,
            .game = game,
            .score_ui = score_ui,
            .settings = games[i].settings,
        };
        lv_obj_t *button = create_button(list, games[i].label, open_game, &selections[i]);
        lv_obj_set_size(button, 148, 36);
        lv_obj_set_style_bg_color(button, lv_color_hex(0x2459a3), 0);
    }

    lv_obj_t *back = create_icon_button(screen, LV_SYMBOL_LEFT, open_home,
                                                   (void *)screens);
    lv_obj_set_size(back, 44, 32);
    lv_obj_align(back, LV_ALIGN_BOTTOM_MID, 0, -14);
}
