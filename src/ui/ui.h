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

/**
 * Breakdown of the UI structure
 * 
 * This will be a stack based UI. There is a central
 * type, Page, that is stored on a stack. Pages are
 * pushed an popped onto the stack. When pushed, it 
 * becomes the current page and gets rendered immediately.
 * When a page is popped, it gets removed from the stack
 * and the new top page is rendered. 
 *
 */

typedef struct Page {
    void (*render)(void);
    struct {
        // No state for now. Could be implemented later on
    } state;
} Page;

void ui_start(struct Channel **channel_list, size_t channel_count);

#endif