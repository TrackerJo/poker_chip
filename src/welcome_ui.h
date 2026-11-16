#ifndef WELCOME_UI_H
#define WELCOME_UI_H

#include "ui_screens.h"


void welcome_ui_create(UiScreens *screens);
bool welcome_ui_dissolve_to(lv_obj_t *destination, uint32_t duration_ms);

#endif
