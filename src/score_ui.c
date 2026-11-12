#include "score_ui.h"
#include <limits.h>
#include <stdio.h>

enum
{
    SCORE_RING_WIDTH = 10,
    HOME_FILL_TIME_MS = 1000,
    SCORE_DRAG_PIXELS_PER_POINT = 16,
};

static void score_face_clicked(lv_event_t *event);
static void score_face_pressed(lv_event_t *event);
static void score_face_pressing(lv_event_t *event);
static void score_face_swiped(lv_event_t *event);
static void minus_icon_held(lv_event_t *event);
static void plus_icon_held(lv_event_t *event);
static void minus_icon_held_ended(lv_event_t *event);
static void plus_icon_held_ended(lv_event_t *event);

static void set_ring_width(void *ring, int32_t width)
{
    lv_obj_set_style_border_width(ring, width, 0);
}

static void cancel_home_hold(ScoreUi *ui)
{
    if (ui->hold_input == NULL)
        return;

    lv_anim_delete(ui->ring, set_ring_width);
    ui->hold_input = NULL;
    set_ring_width(ui->ring, SCORE_RING_WIDTH);
}

static void score_press_ended(lv_event_t *event)
{
    ScoreUi *ui = lv_event_get_user_data(event);
    cancel_home_hold(ui);
    lv_obj_set_hidden(ui->added_label, true);
}

static void home_hold_completed(lv_anim_t *animation)
{
    ScoreUi *ui = lv_anim_get_user_data(animation);
    lv_indev_wait_release(ui->hold_input);
    ui->hold_input = NULL;

    /* Draw the fully filled ring before switching screens. */
    lv_refr_now(lv_obj_get_display(ui->ring));
    ui->screens->open_screen(ui->screens->home);
}

static void home_hold_started(lv_event_t *event)
{
    ScoreUi *ui = lv_event_get_user_data(event);
    lv_indev_t *input = lv_event_get_indev(event);
    if (input == NULL || ui->drag_scoring)
        return;

    ui->hold_input = input;
    lv_anim_t fill;
    lv_anim_init(&fill);
    lv_anim_set_var(&fill, ui->ring);
    lv_anim_set_exec_cb(&fill, set_ring_width);
    lv_anim_set_values(&fill, SCORE_RING_WIDTH, SCORE_DISPLAY_SIZE / 2);
    lv_anim_set_duration(&fill, HOME_FILL_TIME_MS);
    lv_anim_set_path_cb(&fill, lv_anim_path_linear);
    lv_anim_set_user_data(&fill, ui);
    lv_anim_set_completed_cb(&fill, home_hold_completed);
    lv_anim_start(&fill);
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

static void score_ui_refresh(ScoreUi *ui)
{
    const GameState *game = ui->game;
    if (game->active_player >= game->player_count)
    {
        lv_label_set_text(ui->player_label, "");
        show_score(ui, 0);
        return;
    }

    const Player *player = &game->players[game->active_player];
    lv_label_set_text(ui->player_label, player->name);
    show_score(ui, player->score);
}

static void score_screen_loading(lv_event_t *event)
{
    ScoreUi *ui = lv_event_get_user_data(event);
    set_ring_width(ui->ring, SCORE_RING_WIDTH);
    score_ui_refresh(ui);
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
    lv_obj_set_clickable(face, true);
    lv_obj_set_press_lock(face, true);
    lv_obj_set_gesture_bubble(face, false);
    lv_obj_add_event_cb(face, score_face_pressed, LV_EVENT_PRESSED, ui);
    lv_obj_add_event_cb(face, score_face_pressing, LV_EVENT_PRESSING, ui);
    lv_obj_set_size(face, SCORE_DISPLAY_SIZE, SCORE_DISPLAY_SIZE);
    lv_obj_center(face);
    lv_obj_set_style_radius(face, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(face, lv_color_hex(0x729adb), 0);
    lv_obj_set_style_bg_opa(face, LV_OPA_COVER, 0);

    lv_obj_t *ring = lv_obj_create(face);
    lv_obj_remove_style_all(ring);
    lv_obj_set_scrollable(ring, false);
    lv_obj_set_clickable(ring, false);
    lv_obj_set_size(ring, SCORE_DISPLAY_SIZE, SCORE_DISPLAY_SIZE);
    lv_obj_center(ring);
    lv_obj_set_style_radius(ring, LV_RADIUS_CIRCLE, 0);
    set_ring_width(ring, SCORE_RING_WIDTH);
    lv_obj_set_style_border_color(ring, lv_color_hex(0x5084df), 0);

    lv_obj_t *caption = lv_label_create(face);
    lv_obj_set_width(caption, 180);
    lv_label_set_long_mode(caption, LV_LABEL_LONG_MODE_DOTS);
    lv_obj_set_style_text_align(caption, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(caption, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(caption, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_letter_space(caption, 2, 0);
    lv_obj_align(caption, LV_ALIGN_CENTER, 0, -42);

    lv_obj_t *score_row = lv_obj_create(face);
    lv_obj_remove_style_all(score_row);
    lv_obj_set_clickable(score_row, false);
    lv_obj_set_scrollable(score_row, false);
    lv_obj_set_size(score_row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(score_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(score_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(score_row, 8, 0);
    lv_obj_align(score_row, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *minus_icon = lv_label_create(score_row);
    lv_label_set_text(minus_icon, LV_SYMBOL_MINUS);
    lv_obj_set_style_text_color(minus_icon, lv_color_white(), 0);
    lv_obj_set_clickable(minus_icon, true);
    lv_obj_set_gesture_bubble(minus_icon, false);
    lv_obj_set_press_lock(minus_icon, true);
    lv_obj_add_event_cb(minus_icon, minus_icon_held, LV_EVENT_PRESSING, ui);
    lv_obj_add_event_cb(minus_icon, minus_icon_held_ended, LV_EVENT_RELEASED, ui);

    lv_obj_t *score = lv_label_create(score_row);
    lv_obj_set_width(score, LV_SIZE_CONTENT);
    lv_obj_set_style_min_width(score, 96, 0);
    lv_obj_set_style_text_align(score, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(score, lv_color_hex(0xFFFFFF), 0);

    lv_obj_t *plus_icon = lv_label_create(score_row);
    lv_label_set_text(plus_icon, LV_SYMBOL_PLUS);
    lv_obj_set_clickable(plus_icon, true);
    lv_obj_set_gesture_bubble(plus_icon, false);
    lv_obj_set_press_lock(plus_icon, true);
    lv_obj_add_event_cb(plus_icon, plus_icon_held, LV_EVENT_PRESSING, ui);
    lv_obj_add_event_cb(plus_icon, plus_icon_held_ended, LV_EVENT_RELEASED, ui);
    lv_obj_set_style_text_color(plus_icon, lv_color_white(), 0);

    ui->added_label = lv_label_create(face);
    lv_obj_set_style_text_color(ui->added_label, lv_color_hex(0x12274A), 0);
    lv_obj_set_style_bg_color(ui->added_label, lv_color_hex(0xDCE9FF), 0);
    lv_obj_set_style_bg_opa(ui->added_label, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(ui->added_label, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_pad_ver(ui->added_label, 3, 0);
    lv_obj_set_style_pad_hor(ui->added_label, 8, 0);
    lv_obj_align(ui->added_label, LV_ALIGN_CENTER, 0, 38);
    lv_obj_set_hidden(ui->added_label, true);

    lv_obj_move_to_index(ring, -1);
    ui->ring = ring;
    ui->player_label = caption;
    ui->score_label = score;
    score_ui_refresh(ui);
    lv_obj_add_event_cb(face, score_face_clicked, LV_EVENT_SHORT_CLICKED, ui);
    lv_obj_add_event_cb(face, home_hold_started, LV_EVENT_LONG_PRESSED, ui);
    lv_obj_add_event_cb(face, score_press_ended, LV_EVENT_RELEASED, ui);
    lv_obj_add_event_cb(face, score_press_ended, LV_EVENT_PRESS_LOST, ui);
    lv_obj_add_event_cb(face, score_face_swiped, LV_EVENT_GESTURE, ui);
    lv_obj_add_event_cb(screen, score_screen_loading, LV_EVENT_SCREEN_LOAD_START, ui);
    lv_obj_add_event_cb(screen, score_press_ended, LV_EVENT_SCREEN_UNLOAD_START, ui);
}

static void score_face_clicked(lv_event_t *event)
{
    ScoreUi *ui = lv_event_get_user_data(event);
    lv_indev_t *input = lv_event_get_indev(event);
    if (input == NULL)
        return;
    /* LVGL can send a short click after releasing a quick drag. */
    size_t player = ui->game->active_player;
    if (!ui->drag_scoring)
    {
        lv_point_t point;
        lv_indev_get_point(input, &point);
        if (point.x < 120)
        {
            game_subtract_point(ui->game, player);
        }
        else
        {
            game_add_point(ui->game, player);
        }
        show_score(ui, ui->game->players[player].score);
    }
}

static void plus_icon_held(lv_event_t *event)
{
    ScoreUi *ui = lv_event_get_user_data(event);

    printf("Plus HELD: %d\n", ui->plus_hold_delay);
    fflush(stdout);
    size_t player = ui->game->active_player;
    ui->holding_plus_icon = true;
    if (ui->plus_hold_delay == 0)
    {
        game_add_point(ui->game, player);
        show_score(ui, ui->game->players[player].score);
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
    size_t player = ui->game->active_player;
    ui->holding_minus_icon = true;
    if (ui->minus_hold_delay == 0)
    {
        game_subtract_point(ui->game, player);
        show_score(ui, ui->game->players[player].score);
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
}

static void minus_icon_held_ended(lv_event_t *event)
{
    ScoreUi *ui = lv_event_get_user_data(event);

    ui->holding_minus_icon = false;
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

    int32_t radius = SCORE_DISPLAY_SIZE / 2;
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
    if (input == NULL || player >= ui->game->player_count || player >= MAX_PLAYERS || ui->holding_minus_icon || ui->holding_plus_icon)
        return;

    lv_point_t point;
    lv_indev_get_point(input, &point);
    int32_t vertical = ui->press_origin.y - point.y;
    int points = vertical / SCORE_DRAG_PIXELS_PER_POINT;
    if (!ui->drag_scoring)
    {
        int32_t sideways = point.x - ui->press_origin.x;
        if (points == 0 || LV_ABS(vertical) <= LV_ABS(sideways))
            return;

        cancel_home_hold(ui);
        ui->drag_start_score = ui->game->players[player].score;
        ui->drag_scoring = true;
        lv_label_set_text(ui->added_label, "+0");
        lv_obj_set_hidden(ui->added_label, false);
    }

    int score = (int)LV_CLAMP(game_min_score(ui->game),
                              (int64_t)ui->drag_start_score + points, INT_MAX);
    if (game_set_score(ui->game, player, score))
    {
        show_score(ui, score);
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
    cancel_home_hold(ui);

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
