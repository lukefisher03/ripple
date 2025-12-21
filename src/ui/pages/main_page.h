#ifndef MAIN_PAGE_H
#define MAIN_PAGE_H

#include "../../termbox2/termbox2.h"

#include <stdbool.h>

// Forward declare app_state struct
typedef struct app_state app_state;

void main_menu(app_state *app);
static int render_main_menu_selection(int x, int y, bool selected, const void *txt);

#endif