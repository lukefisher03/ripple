#ifndef TB_UI_H
#define TB_UI_H

#include "../list.h"
#include "../parser/xml_rss.h"
#include "pages/local_states.h"
#include "../list.h"
#include <stdbool.h>

typedef enum page_type {
    EXIT_PAGE,
    MAIN_PAGE,
    FEED_PAGE,
    CHANNELS_PAGE,
    CHANNEL_PAGE,
    PREFERENCES_PAGE,
    ARTICLE_PAGE,
    REFRESH_PAGE,
    IMPORT_PAGE,
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

typedef struct {
    // Store global state stuff in here
} app_configuration;

typedef struct {
    page_type   page;
    union {
       // different local state structs 
       article_page_state article_state;
       channel_page_state channel_state;
    };
} local_state;

typedef struct app_state app_state;

typedef void (*page_create)(app_state *, local_state *state);
typedef void (*page_destroy)(void);

typedef struct {
    page_create     create;
    page_destroy    destroy;
} page_handlers;

typedef struct {
    page_type       type;
    local_state     state;
    page_handlers   handlers;
} page;

typedef struct {
    char *new_channel_links_file_path;
} initial_state;

struct app_state {
    page                current_page;
    page                previous_page;
    rss_channel         **channel_list;
    size_t              channel_count;
    app_configuration   config;
    initial_state       init_state;
};


void app_init(app_state *app);
void app_destroy(app_state *app);
void ui_start(initial_state init_state);
void navigate(page_type page_id, app_state *app, local_state state);
#endif