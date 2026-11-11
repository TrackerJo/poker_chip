#include "score_ui.h"
#include <stdio.h>
#include "game_state.h"

void score_ui_create(lv_obj_t *screen, GameState *game)
{
    lv_obj_remove_style_all(screen);
    lv_obj_set_scrollable(screen, false);
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    lv_obj_t *face = lv_obj_create(screen);
    lv_obj_remove_style_all(face);
    lv_obj_set_scrollable(face, false);
    lv_obj_set_clickable(face, false);
    lv_obj_set_size(face, SCORE_DISPLAY_SIZE, SCORE_DISPLAY_SIZE);
    lv_obj_center(face);
    lv_obj_set_style_radius(face, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(face, lv_color_hex(0x10241D), 0);
    lv_obj_set_style_bg_opa(face, LV_OPA_COVER, 0);

    lv_obj_t *ring = lv_obj_create(face);
    lv_obj_remove_style_all(ring);
    lv_obj_set_scrollable(ring, false);
    lv_obj_set_clickable(ring, false);
    lv_obj_set_size(ring, 220, 220);
    lv_obj_center(ring);
    lv_obj_set_style_radius(ring, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(ring, 2, 0);
    lv_obj_set_style_border_color(ring, lv_color_hex(0x4B896D), 0);

    lv_obj_t *caption = lv_label_create(face);
    lv_label_set_text(caption, "SCORING");
    lv_obj_set_style_text_font(caption, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(caption, lv_color_hex(0xAAC9B9), 0);
    lv_obj_set_style_text_letter_space(caption, 2, 0);
    lv_obj_align(caption, LV_ALIGN_CENTER, 0, -32);

    lv_obj_t *score = lv_label_create(face);
    lv_label_set_text_fmt(score, "%d", game->players[game->active_player].score);
    lv_obj_set_style_text_font(score, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(score, lv_color_hex(0xF3FAF5), 0);
    lv_obj_align(score, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_clickable(score, true);
    lv_obj_add_event_cb(score, score_text_clicked, LV_EVENT_CLICKED, game);
}

void score_text_clicked(lv_event_t * event) {
    GameState *game = lv_event_get_user_data(event);
    size_t player = game->active_player;

    if (game_add_point(game, player)) {
        lv_obj_t *score = lv_event_get_target_obj(event);
        lv_label_set_text_fmt(score, "%d", game->players[player].score);
    }
}
