#ifndef TB_UI_H
#define TB_UI_H

// Include all pages
#include "pages/main_page.h"
#include "pages/feeds_page.h"

#include <stdbool.h>

typedef enum page_type {
    MAIN_PAGE,
    FEEDS_PAGE,
    PREFERENCES_PAGE,
    ARTICLE_PAGE,
} page_type;


#define PAGE_COUNT 10
typedef void (*page_renderer)(app_state *);

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

typedef struct app_configuration {
    // Store global state stuff in here
} app_configuration;

typedef struct ui_page {
    // Per page state
    union {
        feeds_page_state    *feed_state;
    };
    generic_list            *page_stack;
    page_type               page_type;
    
    void (*render)(void);
} ui_page;

typedef struct app_state {
    generic_list        *page_stack;
    rss_channel         **channel_list;
    size_t              channel_count;
    app_configuration   config;
} app_state;

void app_init(app_state *app);
void app_destroy(app_state *app);
void ui_start();
void push_page(page_type page_id, app_state *app);
void pop_page(app_state *app);

#endif