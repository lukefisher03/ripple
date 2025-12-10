#ifndef TB_UI_H
#define TB_UI_H

#include <stdbool.h>
#include "termbox2/termbox2.h"

enum MAIN_MENU_OPTIONS {
    OPTION_FEEDS,
    OPTION_PREFERENCES,
    OPTION_FEEDBACK,
    OPTION_EXIT,
};

struct selection {
    int     x;
    int     y;
    char    *text;
    bool    selected;
};

void ui_start();
#endif