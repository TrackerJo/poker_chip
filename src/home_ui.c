#include "home_ui.h"
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
    return bid == values[OH_HELL_TRICKS] ? 10 + bid : bid;
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

static void open_game(lv_event_t *event) {
    const HomeGameContext *context = lv_event_get_user_data(event);
    if (!game_start(context->game, context->settings)) {
        return;
    }
    score_ui_load(context->score_ui);
    context->screens->open_screen(context->screens->add_player, 0);
}

void home_ui_create(UiScreens *screens, GameState *game, ScoreUi *score_ui)
{
    lv_obj_t *screen = screens->home;
    lv_obj_remove_style_all(screen);
    lv_obj_set_scrollable(screen, false);
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    lv_obj_t *face = lv_obj_create(screen);
    lv_obj_remove_style_all(face);
    lv_obj_set_scrollable(face, false);
    lv_obj_set_clickable(face, true);
    lv_obj_set_press_lock(face, true);
    lv_obj_set_gesture_bubble(face, false);
    lv_obj_set_size(face, DISPLAY_SIZE, DISPLAY_SIZE);
    lv_obj_center(face);
    lv_obj_set_style_radius(face, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(face, lv_color_hex(0x729adb), 0);
    lv_obj_set_style_bg_opa(face, LV_OPA_COVER, 0);

    lv_obj_t *ring = lv_obj_create(face);
    lv_obj_remove_style_all(ring);
    lv_obj_set_scrollable(ring, false);
    lv_obj_set_clickable(ring, false);
    lv_obj_set_size(ring, 240, 240);
    lv_obj_center(ring);
    lv_obj_set_style_radius(ring, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(ring, 10, 0);
    lv_obj_set_style_border_color(ring, lv_color_hex(0x5084df), 0);

     lv_obj_t *game_row = lv_obj_create(screen);
    lv_obj_remove_style_all(game_row);
    lv_obj_set_clickable(game_row, false);
    lv_obj_set_scrollable(game_row, false);
    lv_obj_set_size(game_row, DISPLAY_SIZE - 4, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(game_row, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(game_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(game_row, 8, 0);
    lv_obj_set_style_pad_row(game_row, 8, LV_PART_MAIN);
    lv_obj_align(game_row, LV_ALIGN_CENTER, 0, 0);

    static HomeGameContext point_game_context;
    point_game_context = (HomeGameContext){
        .game = game,
        .screens = screens,
        .settings = &point_counter_settings,
        .score_ui = score_ui
    };
    create_button(game_row, "Point Counter", open_game, &point_game_context);


    static HomeGameContext euchre_game_context;
    euchre_game_context = (HomeGameContext){
        .game = game,
        .screens = screens,
        .settings = &euchre_settings,
        .score_ui = score_ui
    };
    create_button(game_row, "Euchre", open_game, &euchre_game_context);

    static HomeGameContext oh_hell_game_context;
    oh_hell_game_context = (HomeGameContext){
        .game = game,
        .screens = screens,
        .settings = &oh_hell_settings,
        .score_ui = score_ui
    };
    create_button(game_row, "Oh Hell", open_game, &oh_hell_game_context);
}
