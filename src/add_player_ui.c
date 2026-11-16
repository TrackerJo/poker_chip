#include "add_player_ui.h"

static const char *const name_keys[] = {
    "A", "B", "C", "D", "E", "F", "G", "H", "I", "\n",
    "J", "K", "L", "M", "N", "O", "P", "Q", "R", "\n",
    "S", "T", "U", "V", "W", "X", "Y", "Z", LV_SYMBOL_BACKSPACE, "",
};

enum {
    NAME_KEY = LV_BUTTONMATRIX_CTRL_WIDTH_1 | LV_BUTTONMATRIX_CTRL_CLICK_TRIG |
               LV_BUTTONMATRIX_CTRL_NO_REPEAT,
};

static const lv_buttonmatrix_ctrl_t name_key_controls[] = {
    NAME_KEY, NAME_KEY, NAME_KEY, NAME_KEY, NAME_KEY, NAME_KEY, NAME_KEY, NAME_KEY, NAME_KEY,
    NAME_KEY, NAME_KEY, NAME_KEY, NAME_KEY, NAME_KEY, NAME_KEY, NAME_KEY, NAME_KEY, NAME_KEY,
    NAME_KEY, NAME_KEY, NAME_KEY, NAME_KEY, NAME_KEY, NAME_KEY, NAME_KEY, NAME_KEY,
    NAME_KEY | LV_BUTTONMATRIX_CTRL_CHECKED,
};

static void show_player_count(AddPlayerUi *ui)
{
    lv_label_set_text_fmt(ui->status, "Players %u/%u",
                         (unsigned)ui->game->player_count, (unsigned)ui->game->settings->max_players);
}

static void refresh_actions(AddPlayerUi *ui) {
    if (ui->game == NULL) return;
    if (ui->game->player_count == 0) {
        lv_obj_set_hidden(ui->edit_icon, true);
        lv_obj_set_hidden(ui->play_icon, true);
    } else {
        lv_obj_set_hidden(ui->edit_icon, false);
        lv_obj_set_hidden(ui->play_icon, false);
    }
    if (ui->game->settings == NULL) return;

    if (ui->game->player_count == ui->game->settings->max_players) {
        lv_obj_set_hidden(ui->add_icon, true);
    } else {
        lv_obj_set_hidden(ui->add_icon, false);
    }
}

static void show_name_entry(AddPlayerUi *ui)
{
    lv_obj_set_hidden(ui->roster_view, true);
    lv_obj_set_hidden(ui->name_view, false);
    lv_obj_clean(ui->roster_list);
    show_player_count(ui);
    refresh_actions(ui);
}

static bool add_entered_player(AddPlayerUi *ui)
{
    if (ui->game->player_count >= ui->game->settings->max_players) {
        lv_label_set_text(ui->status, "Roster full");
        return false;
    }

    const char *name = lv_textarea_get_text(ui->name_input);
    if (!game_add_player(ui->game, name, 0)) {
        lv_label_set_text(ui->status, "Name required");
        return false;
    }

    lv_textarea_set_text(ui->name_input, "");
    show_player_count(ui);
    refresh_actions(ui);
    return true;
}

static void add_player_clicked(lv_event_t *event)
{
    add_entered_player(lv_event_get_user_data(event));
}

static void add_player_screen_loading(lv_event_t *event)
{
    AddPlayerUi *ui = lv_event_get_user_data(event);
    lv_textarea_set_text(ui->name_input, "");
    show_name_entry(ui);
}

static void add_player_hold_started(lv_event_t *event)
{
    AddPlayerUi *ui = lv_event_get_user_data(event);
    home_hold_start(&ui->home_hold, lv_event_get_indev(event));
}

static void add_player_hold_cancelled(lv_event_t *event)
{
    AddPlayerUi *ui = lv_event_get_user_data(event);
    home_hold_cancel(&ui->home_hold);
}

static void start_scoring(lv_event_t *event)
{
    AddPlayerUi *ui = lv_event_get_user_data(event);
    const char *name = lv_textarea_get_text(ui->name_input);
    bool has_room = ui->game->player_count < ui->game->settings->max_players;
    if ((ui->game->player_count == 0 || (has_room && name[0] != '\0')) &&
        !add_entered_player(ui)) {
        return;
    }

    ui->screens->open_screen(ui->screens->score, 0);
}

static lv_obj_t *create_action(lv_obj_t *view, const char *icon, int32_t x,
                          lv_event_cb_t callback, AddPlayerUi *ui)
{
    lv_obj_t *button = create_icon_button(view, icon, callback, ui);
    lv_obj_set_size(button, 44, 28);
    lv_obj_align(button, LV_ALIGN_CENTER, x, 69);
    return button;
}

static void show_empty_roster(AddPlayerUi *ui)
{
    lv_obj_t *label = lv_label_create(ui->roster_list);
    lv_label_set_text(label, "No players yet");
    lv_obj_set_style_text_color(label, lv_color_hex(0x12274A), 0);
}

static void remove_player_clicked(lv_event_t *event)
{
    AddPlayerUi *ui = lv_event_get_user_data(event);
    lv_obj_t *row = lv_obj_get_parent(lv_event_get_current_target_obj(event));
    /* Rows follow the order of players in GameState. */
    size_t index = (size_t)lv_obj_get_index(row);
    game_remove_player(ui->game, ui->game->players[index].id);
    lv_obj_delete(row);
    if (ui->game->player_count == 0) {
        show_empty_roster(ui);
    }

    refresh_actions(ui);
}

static void open_roster(lv_event_t *event)
{
    AddPlayerUi *ui = lv_event_get_user_data(event);
    lv_obj_set_hidden(ui->name_view, true);
    lv_obj_set_hidden(ui->roster_view, false);

    if (ui->game->player_count == 0) {
        show_empty_roster(ui);
    }
    for (size_t index = 0; index < ui->game->player_count; index++) {
        lv_obj_t *row = lv_obj_create(ui->roster_list);
        lv_obj_remove_style_all(row);
        lv_obj_set_clickable(row, false);
        lv_obj_set_scrollable(row, false);
        lv_obj_set_size(row, lv_pct(100), 32);
        lv_obj_set_style_radius(row, 6, 0);
        lv_obj_set_style_bg_color(row, lv_color_hex(0xDCE9FF), 0);
        lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);

        lv_obj_t *name = lv_label_create(row);
        lv_label_set_text(name, ui->game->players[index].name);
        lv_label_set_long_mode(name, LV_LABEL_LONG_MODE_DOTS);
        lv_obj_set_width(name, 114);
        lv_obj_set_style_text_color(name, lv_color_hex(0x12274A), 0);
        lv_obj_align(name, LV_ALIGN_LEFT_MID, 8, 0);

        lv_obj_t *remove = create_icon_button_with_color(row, LV_SYMBOL_TRASH, remove_player_clicked, ui,
                                             lv_color_hex(0x12274A));
        lv_obj_set_size(remove, 40, 32);
        lv_obj_align(remove, LV_ALIGN_RIGHT_MID, 0, 0);
    }
    lv_obj_update_layout(ui->roster_list);
    lv_obj_scroll_to_y(ui->roster_list, 0, LV_ANIM_OFF);
}

static void close_roster(lv_event_t *event)
{
    show_name_entry(lv_event_get_user_data(event));
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

static void create_roster_view(lv_obj_t *screen, AddPlayerUi *ui)
{
    ui->roster_view = create_view(screen);
    lv_obj_t *title = create_label(ui->roster_view, "Edit players");
    lv_obj_align(title, LV_ALIGN_CENTER, 0, -81);

    ui->roster_list = lv_obj_create(ui->roster_view);
    lv_obj_remove_style_all(ui->roster_list);
    lv_obj_set_size(ui->roster_list, 168, 116);
    lv_obj_align(ui->roster_list, LV_ALIGN_CENTER, 0, -5);
    lv_obj_set_flex_flow(ui->roster_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui->roster_list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                         LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(ui->roster_list, 4, 0);
    lv_obj_set_scroll_dir(ui->roster_list, LV_DIR_VER);

    create_action(ui->roster_view, LV_SYMBOL_LEFT, 0, close_roster, ui);
    lv_obj_set_hidden(ui->roster_view, true);
}

void add_player_ui_create(AddPlayerUi *ui, const UiScreens *screens)
{
    ui->screens = screens;
    lv_obj_t *screen = screens->add_player;
    lv_obj_remove_style_all(screen);
    lv_obj_set_scrollable(screen, false);
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    lv_obj_t *face = lv_obj_create(screen);
    lv_obj_remove_style_all(face);
    lv_obj_set_scrollable(face, false);
    lv_obj_set_clickable(face, true);
    lv_obj_set_adv_hittest(face, true);
    lv_obj_set_gesture_bubble(face, false);
    lv_obj_set_size(face, lv_pct(100), lv_pct(100));
    lv_obj_center(face);
    lv_obj_set_style_radius(face, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(face, lv_color_hex(0x729ADB), 0);
    lv_obj_set_style_bg_opa(face, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(face, 10, 0);
    lv_obj_set_style_border_color(face, lv_color_hex(0x5084DF), 0);

    ui->name_view = create_view(screen);
    ui->status = create_label(ui->name_view, "");
    lv_obj_align(ui->status, LV_ALIGN_CENTER, 0, -81);

    ui->name_input = lv_textarea_create(ui->name_view);
    lv_textarea_set_one_line(ui->name_input, true);
    lv_obj_set_scroll_dir(ui->name_input, LV_DIR_HOR);
    lv_obj_set_scrollbar_mode(ui->name_input, LV_SCROLLBAR_MODE_OFF);
    lv_textarea_set_max_length(ui->name_input, (MAX_PLAYER_NAME_LENGTH - 1) / 4);
    lv_textarea_set_placeholder_text(ui->name_input, "Player name");
    lv_obj_set_width(ui->name_input, 156);
    lv_obj_set_style_pad_hor(ui->name_input, 5, 0);
    lv_obj_set_style_pad_ver(ui->name_input, 4, 0);
    lv_obj_align(ui->name_input, LV_ALIGN_CENTER, 0, -55);

    lv_obj_t *keyboard = lv_keyboard_create(ui->name_view);
    lv_keyboard_set_map(keyboard, LV_KEYBOARD_MODE_USER_1, name_keys, name_key_controls);
    lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_USER_1);
    lv_keyboard_set_textarea(keyboard, ui->name_input);
    lv_obj_set_size(keyboard, 192, 86);
    lv_obj_set_style_pad_all(keyboard, 2, 0);
    lv_obj_set_style_pad_gap(keyboard, 2, 0);
    lv_obj_set_style_text_font(keyboard, &lv_font_montserrat_14, LV_PART_ITEMS);
    lv_obj_align(keyboard, LV_ALIGN_CENTER, 0, 5);
    lv_obj_add_event_cb(ui->name_input, add_player_clicked, LV_EVENT_READY, ui);

    ui->add_icon = create_action(ui->name_view, LV_SYMBOL_PLUS, -47, add_player_clicked, ui);
    ui->edit_icon = create_action(ui->name_view, LV_SYMBOL_EDIT, 0, open_roster, ui);
    ui->play_icon = create_action(ui->name_view, LV_SYMBOL_PLAY, 47, start_scoring, ui);
    refresh_actions(ui);

    create_roster_view(screen, ui);
    home_hold_create(&ui->home_hold, screen, screens);
    lv_obj_add_event_cb(face, add_player_hold_started, LV_EVENT_LONG_PRESSED, ui);
    lv_obj_add_event_cb(face, add_player_hold_cancelled, LV_EVENT_RELEASED, ui);
    lv_obj_add_event_cb(face, add_player_hold_cancelled, LV_EVENT_PRESS_LOST, ui);
    lv_obj_add_event_cb(face, add_player_hold_cancelled, LV_EVENT_GESTURE, ui);
    lv_obj_add_event_cb(screen, add_player_screen_loading, LV_EVENT_SCREEN_LOAD_START, ui);
    lv_obj_add_event_cb(screen, add_player_hold_cancelled, LV_EVENT_SCREEN_UNLOAD_START, ui);
}
