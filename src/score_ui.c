#include "score_ui.h"
#include <stdio.h>

LV_FONT_DECLARE(scoreboard_podium_24);
LV_FONT_DECLARE(card_suits_24);
LV_FONT_DECLARE(card_suits_32);
LV_FONT_DECLARE(card_suits_48);

enum
{
    HAND_ERROR_TIME_MS = 2000,
    SCORE_DRAG_PIXELS_PER_POINT = 16,
    ROUND_CONTROL_SIZE = 40,
};

static void score_face_clicked(lv_event_t *event);
static void score_face_pressed(lv_event_t *event);
static void score_face_pressing(lv_event_t *event);
static void score_face_swiped(lv_event_t *event);
static void minus_icon_held(lv_event_t *event);
static void plus_icon_held(lv_event_t *event);
static void minus_icon_held_ended(lv_event_t *event);
static void plus_icon_held_ended(lv_event_t *event);

static bool has_multiple_fields(const ScoreUi *ui)
{
    return ui->game->settings != NULL && ui->game->settings->score_field_count > 1;
}

static void clear_hand_error(lv_timer_t *timer)
{
    ScoreUi *ui = lv_timer_get_user_data(timer);
    ui->hand_error_timer = NULL;
    lv_timer_delete(timer);
    lv_label_set_text(ui->hand_status, "");
    lv_obj_align(ui->hand_status, LV_ALIGN_CENTER, 0, 35);
    if (lv_obj_is_hidden(ui->added_label))
        lv_obj_set_hidden(ui->action_row, false);
}

static void score_press_ended(lv_event_t *event)
{
    ScoreUi *ui = lv_event_get_user_data(event);
    home_hold_cancel(&ui->home_hold);
    if (lv_event_get_code(event) == LV_EVENT_SCREEN_UNLOAD_START && ui->hand_error_timer != NULL)
        clear_hand_error(ui->hand_error_timer);

    lv_obj_set_hidden(ui->added_label, true);
    lv_obj_set_hidden(ui->plus_icon, false);
    lv_obj_set_hidden(ui->minus_icon, false);
    lv_obj_set_hidden(ui->action_row, ui->hand_error_timer != NULL);
    lv_obj_set_hidden(ui->field_tabs, !has_multiple_fields(ui));

    lv_obj_set_hidden(ui->hand_status, !has_multiple_fields(ui));
}

static void home_hold_started(lv_event_t *event)
{
    ScoreUi *ui = lv_event_get_user_data(event);
    if (!ui->drag_scoring)
        home_hold_start(&ui->home_hold, lv_event_get_indev(event));
}

static void show_score(ScoreUi *ui, int score)
{
    bool large_score = score <= -10000 || score >= 10000;
    const lv_font_t *font = large_score ? &lv_font_montserrat_14 : &lv_font_montserrat_48;
    if (lv_obj_get_style_text_font(ui->score_label, 0) != font)
    {
        lv_obj_set_style_text_font(ui->score_label, font, 0);
    }
    lv_label_set_text_fmt(ui->score_label, "%d", score);
}

static void refresh_score_value(ScoreUi *ui)
{
    const GameState *game = ui->game;
    int value;
    if (game_get_field(game, game->active_player, ui->active_field, &value))
    {
        show_score(ui, value);
    }
    else
    {
        lv_obj_set_style_text_font(ui->score_label, &lv_font_montserrat_48, 0);
        lv_label_set_text(ui->score_label, "-");
    }

    if (has_multiple_fields(ui) && ui->hand_error_timer == NULL &&
        lv_label_get_text(ui->hand_status)[0] != '\0')
        lv_label_set_text(ui->hand_status, "");
}

static void score_ui_refresh(ScoreUi *ui)
{
    const GameState *game = ui->game;
    bool multiple_fields = has_multiple_fields(ui);
    lv_obj_set_hidden(ui->field_tabs, !multiple_fields);
    lv_obj_set_hidden(ui->hand_status, !multiple_fields);
    lv_obj_set_hidden(ui->finish_hand_button, !multiple_fields);
    lv_obj_align(ui->action_row, LV_ALIGN_CENTER, 0, 52);
    if (multiple_fields)
    {
        for (size_t field = 0; field < MAX_SCORE_FIELDS; field++)
        {
            lv_obj_t *tab = lv_obj_get_child(ui->field_tabs, (int32_t)field);
            bool available = field < game->settings->score_field_count;
            lv_obj_set_hidden(tab, !available);
            if (available)
            {
                bool selected = field == ui->active_field;
                lv_obj_t *label = lv_obj_get_child(tab, 0);
                lv_obj_set_state(tab, LV_STATE_CHECKED, selected);
                lv_obj_set_style_text_color(label, selected ? lv_color_hex(0x12274A) : lv_color_white(), 0);
                lv_label_set_text(label, game->settings->score_fields[field].label);
            }
        }
    }

    if (game->active_player >= game->player_count)
    {
        lv_label_set_text(ui->player_label, "");
        show_score(ui, 0);
        return;
    }

    const Player *player = &game->players[game->active_player];
    lv_label_set_text(ui->player_label, player->name);
    refresh_score_value(ui);
}

static void select_score_field(lv_event_t *event)
{
    ScoreUi *ui = lv_event_get_user_data(event);
    size_t field = (size_t)lv_obj_get_index(lv_event_get_current_target_obj(event));
    if (!has_multiple_fields(ui) || field >= ui->game->settings->score_field_count ||
        field == ui->active_field)
        return;

    ui->active_field = field;
    score_ui_refresh(ui);
}

static void create_field_tabs(ScoreUi *ui)
{
    ui->field_tabs = lv_obj_create(ui->score_view);
    lv_obj_remove_style_all(ui->field_tabs);
    lv_obj_set_clickable(ui->field_tabs, false);
    lv_obj_set_scrollable(ui->field_tabs, false);
    lv_obj_set_size(ui->field_tabs, 158, 30);
    lv_obj_set_style_pad_all(ui->field_tabs, 3, 0);
    lv_obj_set_style_pad_column(ui->field_tabs, 2, 0);
    lv_obj_set_style_radius(ui->field_tabs, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(ui->field_tabs, lv_color_hex(0x355B91), 0);
    lv_obj_set_style_bg_opa(ui->field_tabs, LV_OPA_COVER, 0);
    lv_obj_set_flex_flow(ui->field_tabs, LV_FLEX_FLOW_ROW);
    lv_obj_align(ui->field_tabs, LV_ALIGN_CENTER, 0, -67);

    for (size_t field = 0; field < MAX_SCORE_FIELDS; field++)
    {
        lv_obj_t *tab = create_icon_button(ui->field_tabs, "", select_score_field, ui);
        lv_obj_set_gesture_bubble(tab, false);
        lv_obj_set_height(tab, lv_pct(100));
        lv_obj_set_flex_grow(tab, 1);
        lv_obj_set_style_radius(tab, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(tab, lv_color_hex(0xE8F0FF), 0);
        lv_obj_set_style_bg_opa(tab, LV_OPA_30, LV_STATE_PRESSED);
        lv_obj_set_style_bg_opa(tab, LV_OPA_COVER, LV_STATE_CHECKED);

        lv_obj_t *label = lv_obj_get_child(tab, 0);
        lv_obj_set_width(label, lv_pct(100));
        lv_label_set_long_mode(label, LV_LABEL_LONG_MODE_DOTS);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    }
}

static void show_hand_error(ScoreUi *ui, const char *message)
{
    lv_label_set_text(ui->hand_status, message);
    ui->hand_error_timer = lv_timer_create(clear_hand_error, HAND_ERROR_TIME_MS, ui);
    if (ui->hand_error_timer != NULL)
    {
        lv_obj_align_to(ui->hand_status, ui->action_row, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_hidden(ui->action_row, true);
    }
}

static void finish_hand_clicked(lv_event_t *event)
{
    ScoreUi *ui = lv_event_get_user_data(event);
    if (ui->hand_error_timer != NULL)
        return;

    size_t missing_player, missing_field;
    switch (game_finish_hand(ui->game, &missing_player, &missing_field))
    {
    case FINISH_HAND_OK:
        ui->active_field = 0;
        score_ui_refresh(ui);
        lv_label_set_text(ui->hand_status, "Hand saved");
        break;
    case FINISH_HAND_MISSING_FIELD:
        ui->game->active_player = missing_player;
        ui->active_field = missing_field;
        score_ui_refresh(ui);
        show_hand_error(ui, ui->game->settings->score_fields[missing_field].min_value == 0
                                ? "Enter a value (- for 0)"
                                : "Enter a value");
        break;
    case FINISH_HAND_SCORE_OUT_OF_RANGE:
        show_hand_error(ui, "Total out of range");
        break;
    case FINISH_HAND_NO_MEMORY:
        show_hand_error(ui, "Out of memory");
        break;
    case FINISH_HAND_UNAVAILABLE:
        show_hand_error(ui, "No hand to finish");
        break;
    }
}

static void show_score_view(ScoreUi *ui)
{
    lv_obj_set_hidden(ui->leaderboard_view, true);
    lv_obj_set_hidden(ui->trump_view, true);
    lv_obj_set_hidden(ui->score_view, false);
    lv_obj_clean(ui->leaderboard_list);
}

static void score_screen_loading(lv_event_t *event)
{
    ScoreUi *ui = lv_event_get_user_data(event);
    show_score_view(ui);
    home_hold_cancel(&ui->home_hold);
    score_ui_refresh(ui);
}

static void create_action(lv_obj_t *view, const char *icon, int32_t x,
                          lv_event_cb_t callback, ScoreUi *ui)
{
    lv_obj_t *button = create_icon_button(view, icon, callback, ui);
    lv_obj_set_size(button, 44, 28);
    lv_obj_align(button, LV_ALIGN_CENTER, x, 69);
}

static lv_obj_t *create_view(lv_obj_t *screen)
{
    lv_obj_t *view = lv_obj_create(screen);
    lv_obj_remove_style_all(view);
    lv_obj_set_size(view, lv_pct(100), lv_pct(100));
    lv_obj_set_clickable(view, false);
    lv_obj_set_scrollable(view, false);
    return view;
}

static void close_leaderboard_view(lv_event_t *event)
{
    show_score_view(lv_event_get_user_data(event));
}

static void create_leaderboard_view(lv_obj_t *screen, ScoreUi *ui)
{
    ui->leaderboard_view = create_view(screen);
    lv_obj_set_clickable(ui->leaderboard_view, true);
    lv_obj_set_press_lock(ui->leaderboard_view, true);
    lv_obj_add_event_cb(ui->leaderboard_view, score_face_pressed, LV_EVENT_PRESSED, ui);
    lv_obj_add_event_cb(ui->leaderboard_view, home_hold_started, LV_EVENT_LONG_PRESSED, ui);
    lv_obj_add_event_cb(ui->leaderboard_view, score_press_ended, LV_EVENT_RELEASED, ui);
    lv_obj_add_event_cb(ui->leaderboard_view, score_press_ended, LV_EVENT_PRESS_LOST, ui);
    lv_obj_add_event_cb(ui->leaderboard_view, score_press_ended, LV_EVENT_SCROLL_BEGIN, ui);

    lv_obj_t *title = create_label(ui->leaderboard_view, "Leaderboard");
    lv_obj_align(title, LV_ALIGN_CENTER, 0, -81);

    ui->leaderboard_list = lv_obj_create(ui->leaderboard_view);
    lv_obj_remove_style_all(ui->leaderboard_list);
    lv_obj_set_size(ui->leaderboard_list, 168, 116);
    lv_obj_align(ui->leaderboard_list, LV_ALIGN_CENTER, 0, -5);
    lv_obj_set_flex_flow(ui->leaderboard_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui->leaderboard_list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(ui->leaderboard_list, 4, 0);
    lv_obj_set_scroll_dir(ui->leaderboard_list, LV_DIR_VER);
    lv_obj_set_event_bubble(ui->leaderboard_list, true);

    create_action(ui->leaderboard_view, LV_SYMBOL_LEFT, 0, close_leaderboard_view, ui);
    lv_obj_set_hidden(ui->leaderboard_view, true);
}

static void select_player(lv_event_t *event)
{
    ScoreUi *ui = lv_event_get_user_data(event);
    lv_obj_t *row = lv_event_get_current_target_obj(event);
    const Player *player = lv_obj_get_user_data(row);

    ui->game->active_player = (size_t)(player - ui->game->players);
    show_score_view(ui);
    score_ui_refresh(ui);
}

static void open_leaderboard_view(lv_event_t *event)
{
    ScoreUi *ui = lv_event_get_user_data(event);
    lv_obj_set_hidden(ui->score_view, true);
    lv_obj_set_hidden(ui->leaderboard_view, false);

    size_t player_count = ui->game->player_count;
    Player *ranked_players[ui->game->settings->max_players];
    for (size_t index = 0; index < player_count; index++)
    {
        Player *player = &ui->game->players[index];
        size_t rank = index;
        while (rank > 0 && ranked_players[rank - 1]->score < player->score)
        {
            ranked_players[rank] = ranked_players[rank - 1];
            rank--;
        }
        ranked_players[rank] = player;
    }

    for (size_t index = 0; index < player_count; index++)
    {
        Player *player = ranked_players[index];
        lv_obj_t *row = lv_obj_create(ui->leaderboard_list);
        lv_obj_remove_style_all(row);
        lv_obj_set_clickable(row, true);
        lv_obj_set_scrollable(row, false);
        lv_obj_set_size(row, lv_pct(100), 32);
        lv_obj_set_style_radius(row, 6, 0);
        lv_obj_set_style_bg_color(row, lv_color_hex(0xDCE9FF), 0);
        lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
        lv_obj_set_user_data(row, player);
        lv_obj_add_event_cb(row, select_player, LV_EVENT_CLICKED, ui);

        lv_obj_t *name = lv_label_create(row);
        lv_label_set_text(name, player->name);
        lv_label_set_long_mode(name, LV_LABEL_LONG_MODE_DOTS);
        lv_obj_set_width(name, 114);
        lv_obj_set_style_text_color(name, lv_color_hex(0x12274A), 0);
        lv_obj_align(name, LV_ALIGN_LEFT_MID, 8, 0);

        lv_obj_t *score = lv_label_create(row);
        lv_label_set_text_fmt(score, "%d", player->score);
        lv_label_set_long_mode(score, LV_LABEL_LONG_MODE_DOTS);
        lv_obj_set_style_text_color(score, lv_color_hex(0x12274A), 0);
        lv_obj_set_width(score, LV_SIZE_CONTENT);
        lv_obj_align(score, LV_ALIGN_RIGHT_MID, -8, 0);
    }
    lv_obj_update_layout(ui->leaderboard_list);
    lv_obj_scroll_to_y(ui->leaderboard_list, 0, LV_ANIM_OFF);
}

static void close_trump_view(lv_event_t *event)
{
    show_score_view(lv_event_get_user_data(event));
}

static void open_trump_view(lv_event_t *event)
{
    ScoreUi *ui = lv_event_get_user_data(event);
    lv_obj_set_hidden(ui->score_view, true);
    lv_obj_set_hidden(ui->trump_view, false);
}

static void select_trump_suit(lv_event_t *event)
{
    ScoreUi *ui = lv_event_get_user_data(event);
    lv_obj_t *suit_button = lv_event_get_current_target_obj(event);
    lv_obj_t *suit_label = lv_obj_get_child(suit_button, 0);
    lv_obj_t *trump_label = lv_obj_get_child(ui->trump_icon, 0);
    lv_label_set_text(trump_label, lv_label_get_text(suit_label));
    lv_obj_set_style_text_color(trump_label,
                                lv_obj_get_style_text_color(suit_label, LV_PART_MAIN),
                                LV_PART_MAIN);
    show_score_view(ui);
}

static void create_trump_view(lv_obj_t *screen, ScoreUi *ui)
{
    ui->trump_view = create_view(screen);
    lv_obj_set_clickable(ui->trump_view, true);
    lv_obj_set_press_lock(ui->trump_view, true);
    lv_obj_add_event_cb(ui->trump_view, score_face_pressed, LV_EVENT_PRESSED, ui);
    lv_obj_add_event_cb(ui->trump_view, home_hold_started, LV_EVENT_LONG_PRESSED, ui);
    lv_obj_add_event_cb(ui->trump_view, score_press_ended, LV_EVENT_RELEASED, ui);
    lv_obj_add_event_cb(ui->trump_view, score_press_ended, LV_EVENT_PRESS_LOST, ui);
    lv_obj_add_event_cb(ui->trump_view, score_press_ended, LV_EVENT_SCROLL_BEGIN, ui);

    lv_obj_t *title = create_label(ui->trump_view, "Trump Suit");
    lv_obj_align(title, LV_ALIGN_CENTER, 0, -81);

    lv_obj_t *red_row = lv_obj_create(ui->trump_view);
    lv_obj_remove_style_all(red_row);
    lv_obj_set_clickable(red_row, false);
    lv_obj_set_scrollable(red_row, false);
    lv_obj_set_size(red_row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(red_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(red_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(red_row, 8, 0);
    lv_obj_align(red_row, LV_ALIGN_CENTER, 0, -26);

    lv_obj_t *hearts = create_icon_button_with_color(
        red_row, "♥", select_trump_suit, ui, lv_color_hex(0xD32F2F));
    lv_obj_set_style_text_font(hearts, &card_suits_48, LV_PART_MAIN);
    lv_obj_set_size(hearts, 48, 48);

    lv_obj_t *diamonds = create_icon_button_with_color(
        red_row, "♦", select_trump_suit, ui, lv_color_hex(0xD32F2F));
    lv_obj_set_style_text_font(diamonds, &card_suits_48, LV_PART_MAIN);
    lv_obj_set_size(diamonds, 48, 48);

    lv_obj_t *black_row = lv_obj_create(ui->trump_view);
    lv_obj_remove_style_all(black_row);
    lv_obj_set_clickable(black_row, false);
    lv_obj_set_scrollable(black_row, false);
    lv_obj_set_size(black_row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(black_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(black_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(black_row, 8, 0);
    lv_obj_align(black_row, LV_ALIGN_CENTER, 0, 26);

    lv_obj_t *spades = create_icon_button_with_color(
        black_row, "♠", select_trump_suit, ui, lv_color_hex(0x000000));
    lv_obj_set_style_text_font(spades, &card_suits_48, LV_PART_MAIN);
    lv_obj_set_size(spades, 48, 48);

    lv_obj_t *clubs = create_icon_button_with_color(
        black_row, "♣", select_trump_suit, ui, lv_color_hex(0x000000));
    lv_obj_set_style_text_font(clubs, &card_suits_48, LV_PART_MAIN);
    lv_obj_set_size(clubs, 48, 48);

    create_action(ui->trump_view, LV_SYMBOL_LEFT, 0, close_trump_view, ui);
    lv_obj_set_hidden(ui->trump_view, true);
}

static void draw_euchre_buttons(ScoreUi *ui)
{
    ui->trump_icon = create_icon_button_with_color(
        ui->action_row, "♥", open_trump_view, ui, lv_color_hex(0xD32F2F));
    lv_obj_set_style_text_font(ui->trump_icon, &card_suits_24, LV_PART_MAIN);
    lv_obj_set_size(ui->trump_icon, 44, 44);
}

static void style_round_control(lv_obj_t *button)
{
    lv_obj_set_size(button, ROUND_CONTROL_SIZE, ROUND_CONTROL_SIZE);
    lv_obj_set_gesture_bubble(button, false);
    lv_obj_set_press_lock(button, true);
    lv_obj_set_style_radius(button, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0xE8F0FF), 0);
    lv_obj_set_style_bg_opa(button, LV_OPA_40, LV_STATE_PRESSED);
}

static lv_obj_t *create_score_step_button(lv_obj_t *parent, const char *symbol,
                                          lv_event_cb_t holding, lv_event_cb_t released,
                                          ScoreUi *ui)
{
    lv_obj_t *button = lv_button_create(parent);
    lv_obj_remove_style_all(button);
    style_round_control(button);
    lv_obj_t *label = create_label(button, symbol);
    lv_obj_set_style_text_letter_space(label, 0, 0);
    lv_obj_center(label);
    lv_obj_add_event_cb(button, holding, LV_EVENT_PRESSING, ui);
    lv_obj_add_event_cb(button, released, LV_EVENT_RELEASED, ui);
    lv_obj_add_event_cb(button, released, LV_EVENT_PRESS_LOST, ui);
    return button;
}

void score_ui_load(ScoreUi *ui)
{
    ui->active_field = 0;
    if (ui->trump_icon != NULL)
    {
        lv_obj_delete(ui->trump_icon);
        ui->trump_icon = NULL;
    }
    if (ui->game->settings != NULL && ui->game->settings->kind == EUCHRE)
    {
        draw_euchre_buttons(ui);
    }
}

void score_ui_create(ScoreUi *ui, const UiScreens *screens)
{
    ui->screens = screens;
    lv_obj_t *screen = screens->score;
    lv_obj_remove_style_all(screen);
    lv_obj_set_scrollable(screen, false);
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    lv_obj_t *face = lv_obj_create(screen);
    lv_obj_remove_style_all(face);
    lv_obj_set_scrollable(face, false);
    lv_obj_set_clickable(face, false);
    lv_obj_set_size(face, lv_pct(100), lv_pct(100));
    lv_obj_center(face);
    lv_obj_set_style_radius(face, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(face, lv_color_hex(0x729ADB), 0);
    lv_obj_set_style_bg_opa(face, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(face, 10, 0);
    lv_obj_set_style_border_color(face, lv_color_hex(0x5084DF), 0);

    ui->score_view = create_view(screen);
    lv_obj_set_clickable(ui->score_view, true);
    lv_obj_set_press_lock(ui->score_view, true);
    lv_obj_set_gesture_bubble(ui->score_view, false);

    create_field_tabs(ui);

    lv_obj_t *caption = create_label(ui->score_view, NULL);
    lv_obj_set_width(caption, 180);
    lv_label_set_long_mode(caption, LV_LABEL_LONG_MODE_DOTS);
    lv_obj_set_style_text_align(caption, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(caption, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(caption, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_letter_space(caption, 2, 0);
    lv_obj_align(caption, LV_ALIGN_CENTER, 0, -38);

    lv_obj_t *score_row = lv_obj_create(ui->score_view);
    lv_obj_remove_style_all(score_row);
    lv_obj_set_clickable(score_row, false);
    lv_obj_set_scrollable(score_row, false);
    lv_obj_set_size(score_row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(score_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(score_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(score_row, 8, 0);
    lv_obj_align(score_row, LV_ALIGN_CENTER, 0, 0);

    ui->minus_icon = create_score_step_button(score_row, LV_SYMBOL_MINUS, minus_icon_held,
                                              minus_icon_held_ended, ui);

    lv_obj_t *score = lv_label_create(score_row);
    lv_obj_set_width(score, LV_SIZE_CONTENT);
    lv_obj_set_style_min_width(score, 96, 0);
    lv_obj_set_style_text_align(score, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(score, lv_color_hex(0xFFFFFF), 0);

    ui->hand_status = create_label(ui->score_view, "");
    lv_obj_set_style_text_letter_space(ui->hand_status, 0, 0);
    lv_obj_align(ui->hand_status, LV_ALIGN_CENTER, 0, 35);

    ui->plus_icon = create_score_step_button(score_row, LV_SYMBOL_PLUS, plus_icon_held,
                                             plus_icon_held_ended, ui);

    ui->added_label = lv_label_create(ui->score_view);
    lv_obj_set_style_text_color(ui->added_label, lv_color_hex(0x12274A), 0);
    lv_obj_set_style_bg_color(ui->added_label, lv_color_hex(0xDCE9FF), 0);
    lv_obj_set_style_bg_opa(ui->added_label, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(ui->added_label, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_pad_ver(ui->added_label, 3, 0);
    lv_obj_set_style_pad_hor(ui->added_label, 8, 0);
    lv_obj_align(ui->added_label, LV_ALIGN_CENTER, 0, 38);
    lv_obj_set_hidden(ui->added_label, true);

    ui->action_row = lv_obj_create(ui->score_view);
    lv_obj_remove_style_all(ui->action_row);
    lv_obj_set_clickable(ui->action_row, false);
    lv_obj_set_scrollable(ui->action_row, false);
    lv_obj_set_size(ui->action_row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(ui->action_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ui->action_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(ui->action_row, 8, 0);


    ui->leaderboard_icon = create_icon_button(
        ui->action_row, "\xEE\x80\x80", open_leaderboard_view, ui);
    lv_obj_set_style_text_font(lv_obj_get_child(ui->leaderboard_icon, 0), &scoreboard_podium_24, 0);
    style_round_control(ui->leaderboard_icon);

    ui->finish_hand_button = create_button(ui->action_row, "Finish Hand", finish_hand_clicked, ui);
    lv_obj_set_size(ui->finish_hand_button, 112, ROUND_CONTROL_SIZE);
    lv_obj_set_gesture_bubble(ui->finish_hand_button, false);
    lv_obj_set_style_radius(ui->finish_hand_button, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(ui->finish_hand_button, lv_color_hex(0x152E52), 0);
    lv_obj_set_style_bg_color(ui->finish_hand_button, lv_color_hex(0x234774), LV_STATE_PRESSED);
    lv_obj_set_style_shadow_width(ui->finish_hand_button, 0, 0);

    create_leaderboard_view(screen, ui);
    create_trump_view(screen, ui);

    home_hold_create(&ui->home_hold, screen, screens);
    ui->player_label = caption;
    ui->score_label = score;

    score_ui_refresh(ui);

    lv_obj_add_event_cb(ui->score_view, score_face_pressed, LV_EVENT_PRESSED, ui);
    lv_obj_add_event_cb(ui->score_view, score_face_pressing, LV_EVENT_PRESSING, ui);
    lv_obj_add_event_cb(ui->score_view, score_face_clicked, LV_EVENT_SHORT_CLICKED, ui);
    lv_obj_add_event_cb(ui->score_view, home_hold_started, LV_EVENT_LONG_PRESSED, ui);
    lv_obj_add_event_cb(ui->score_view, score_press_ended, LV_EVENT_RELEASED, ui);
    lv_obj_add_event_cb(ui->score_view, score_press_ended, LV_EVENT_PRESS_LOST, ui);
    lv_obj_add_event_cb(ui->score_view, score_face_swiped, LV_EVENT_GESTURE, ui);
    lv_obj_add_event_cb(screen, score_screen_loading, LV_EVENT_SCREEN_LOAD_START, ui);
    lv_obj_add_event_cb(screen, score_press_ended, LV_EVENT_SCREEN_UNLOAD_START, ui);
}

static void change_score_field(ScoreUi *ui, int change)
{
    GameState *game = ui->game;
    size_t player = game->active_player;
    if (player >= game->player_count)
        return;

    const ScoreField *field = &game->settings->score_fields[ui->active_field];
    int current = 0;
    game_get_field(game, player, ui->active_field, &current);
    int value = (int)LV_CLAMP(field->min_value, (int64_t)current + change, field->max_value);
    if (game_set_field(game, player, ui->active_field, value))
        refresh_score_value(ui);
}

static void score_face_clicked(lv_event_t *event)
{
    ScoreUi *ui = lv_event_get_user_data(event);
    lv_indev_t *input = lv_event_get_indev(event);
    if (input == NULL)
        return;
    /* LVGL can send a short click after releasing a quick drag. */
    if (!ui->drag_scoring)
    {
        lv_point_t point;
        lv_indev_get_point(input, &point);
        change_score_field(ui, point.x < DISPLAY_SIZE / 2 ? -1 : 1);
    }
}

static void plus_icon_held(lv_event_t *event)
{
    ScoreUi *ui = lv_event_get_user_data(event);

    printf("Plus HELD: %d\n", ui->plus_hold_delay);
    fflush(stdout);
    ui->holding_plus_icon = true;
    if (ui->plus_hold_delay == 0)
    {
        change_score_field(ui, 1);
        ui->plus_hold_delay = 5;
    }
    else
    {
        ui->plus_hold_delay -= 1;
    }
}

static void minus_icon_held(lv_event_t *event)
{
    ScoreUi *ui = lv_event_get_user_data(event);

    printf("MINUS HELD: %d\n", ui->minus_hold_delay);
    fflush(stdout);
    ui->holding_minus_icon = true;
    if (ui->minus_hold_delay == 0)
    {
        change_score_field(ui, -1);
        ui->minus_hold_delay = 5;
    }
    else
    {
        ui->minus_hold_delay -= 1;
    }
}

static void plus_icon_held_ended(lv_event_t *event)
{
    ScoreUi *ui = lv_event_get_user_data(event);
    ui->holding_plus_icon = false;
    ui->plus_hold_delay = 0;
}

static void minus_icon_held_ended(lv_event_t *event)
{
    ScoreUi *ui = lv_event_get_user_data(event);

    ui->holding_minus_icon = false;
    ui->minus_hold_delay = 0;
}

static void score_face_pressed(lv_event_t *event)
{
    ScoreUi *ui = lv_event_get_user_data(event);
    lv_indev_t *input = lv_event_get_indev(event);
    if (input == NULL)
        return;

    lv_point_t point;
    lv_indev_get_point(input, &point);
    lv_area_t bounds;
    lv_obj_get_coords(lv_event_get_current_target_obj(event), &bounds);

    int32_t radius = DISPLAY_SIZE / 2;
    int32_t x = point.x - bounds.x1 - radius;
    int32_t y = point.y - bounds.y1 - radius;
    if (x * x + y * y > radius * radius)
    {
        lv_indev_wait_release(input);
        return;
    }

    ui->press_origin = point;
    ui->drag_scoring = false;
}

static void score_face_pressing(lv_event_t *event)
{
    ScoreUi *ui = lv_event_get_user_data(event);
    size_t player = ui->game->active_player;
    lv_indev_t *input = lv_event_get_indev(event);
    if (input == NULL || player >= ui->game->player_count || ui->holding_minus_icon || ui->holding_plus_icon)
        return;

    const ScoreField *field = &ui->game->settings->score_fields[ui->active_field];

    lv_point_t point;
    lv_indev_get_point(input, &point);
    int32_t vertical = ui->press_origin.y - point.y;
    int points = vertical / SCORE_DRAG_PIXELS_PER_POINT;
    if (!ui->drag_scoring)
    {
        int32_t sideways = point.x - ui->press_origin.x;
        if (points == 0 || LV_ABS(vertical) <= LV_ABS(sideways))
            return;

        home_hold_cancel(&ui->home_hold);
        ui->drag_start_score = LV_CLAMP(field->min_value, 0, field->max_value);
        game_get_field(ui->game, player, ui->active_field, &ui->drag_start_score);
        ui->drag_scoring = true;
        lv_label_set_text(ui->added_label, "+0");
        lv_obj_set_hidden(ui->added_label, false);
        lv_obj_set_hidden(ui->plus_icon, true);
        lv_obj_set_hidden(ui->minus_icon, true);
        lv_obj_set_hidden(ui->action_row, true);
        lv_obj_set_hidden(ui->hand_status, true);
        lv_obj_set_hidden(ui->field_tabs, true);
    }

    int score = (int)LV_CLAMP(field->min_value, (int64_t)ui->drag_start_score + points,
                              field->max_value);
    if (game_set_field(ui->game, player, ui->active_field, score))
    {
        refresh_score_value(ui);
        int change = score - ui->drag_start_score;
        lv_label_set_text_fmt(ui->added_label, change < 0 ? "%d" : "+%d", change);
    }
}

static void score_face_swiped(lv_event_t *event)
{
    ScoreUi *ui = lv_event_get_user_data(event);
    if (ui->drag_scoring)
        return;

    lv_indev_t *input = lv_indev_active();
    home_hold_cancel(&ui->home_hold);

    switch (lv_indev_get_gesture_dir(input))
    {
    case LV_DIR_LEFT:
        printf("Swiped left\n");
        game_rotate_active_player_left(ui->game);
        break;
    case LV_DIR_RIGHT:
        printf("Swiped right\n");
        game_rotate_active_player_right(ui->game);
        break;
    default:
        lv_indev_wait_release(input);
        return;
    }

    fflush(stdout);
    score_ui_refresh(ui);
    lv_indev_wait_release(input);
}
