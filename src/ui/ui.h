#ifndef TB_UI_H
#define TB_UI_H

#include "../list.h"
#include "../parser/xml_rss.h"

#include <stdbool.h>

typedef enum page_type {
    MAIN_PAGE,
    FEEDS_PAGE,
    PREFERENCES_PAGE,
    ARTICLE_PAGE,
} page_type;

#define PAGE_COUNT 10

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

typedef struct page_handlers page_handlers;

typedef struct app_state {
    generic_list        *page_stack;
    page_handlers       *current_page_handlers; 
    rss_channel         **channel_list;
    size_t              channel_count;
    app_configuration   config;
} app_state;

typedef void (*page_create)(app_state *);
typedef void (*page_destroy)(void);

typedef struct page_handlers {
    page_create     create;
    page_destroy    destroy;
} page_handlers;

void app_init(app_state *app);
void app_destroy(app_state *app);
void ui_start();
void push_page(page_type page_id, app_state *app);
#endif