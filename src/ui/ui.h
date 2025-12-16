#ifndef TB_UI_H
#define TB_UI_H

#include "../parser/xml_rss.h"

#include <stdbool.h>

typedef enum MAIN_MENU_OPTIONS {
    OPTION_FEEDS,
    OPTION_PREFERENCES,
    OPTION_FEEDBACK,
    OPTION_EXIT,
} MAIN_MENU_OPTIONS;

typedef struct Page {
    void (*render)(void);
    struct {
        // No state for now. Could be implemented later on
    } state;
} Page;

void ui_start(struct Channel **channel_list, size_t channel_count);
#endif