#ifndef MAIN_PAGE_H
#define MAIN_PAGE_H

#include "../../termbox2/termbox2.h"

#include <stdbool.h>

int main_menu(void);
static int render_main_menu_selection(int x, int y, bool selected, const void *txt);

#endif