#ifndef TB_UI_H
#define TB_UI_H

#include <stdbool.h>
#include "termbox2/termbox2.h"
#include "parser/xml_rss.h"

struct channel_column_widths {
    size_t  title_width;
    size_t  author_width;
    size_t  pub_date_width;
};

enum MAIN_MENU_OPTIONS {
    OPTION_FEEDS,
    OPTION_PREFERENCES,
    OPTION_FEEDBACK,
    OPTION_EXIT,
};

void ui_start(struct channel **channel_list, size_t channel_count);
#endif