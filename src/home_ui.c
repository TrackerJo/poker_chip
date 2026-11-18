#include "home_ui.h"
#include "components.h"

LV_IMAGE_DECLARE(point_counter_icon);

static void open_game_select(lv_event_t *event)
{
    const UiScreens *screens = lv_event_get_user_data(event);
    screens->open_screen(screens->game_select, 0);
}

void home_ui_create(const UiScreens *screens)
{
    lv_obj_t *screen = screens->home;
    create_round_screen_background(screen);
    lv_obj_t *icon = create_app_icon(screen, &point_counter_icon, open_game_select, (void *)screens);
    lv_obj_center(icon);
}
