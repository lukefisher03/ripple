// Prevent redefinitions by putting this before
// the definition of TB_IMPL
#include "ui.h"
#include "ui_utils.h" 
#include "../list.h"

#include "pages/main_page.h"
#include "pages/feeds_page.h"
#include "pages/article_page.h"

#define TB_IMPL

// Now after TB_IMPL is defined, the definitions will be included here.
#include "../termbox2/termbox2.h"
#include "../arena.h"

#include <stdarg.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>
#include <string.h>


static page_handlers page_handlers_table[PAGE_COUNT] = {
    [MAIN_PAGE] = {
        .create = main_menu,
        .destroy = NULL,
    },
    [FEEDS_PAGE] = {
        .create = feed_reader,
        .destroy = feed_reader_destroy,
    }, 
    [ARTICLE_PAGE] = {
        .create = article_page,
        .destroy = NULL,
    }
};

// ------ Main UI Call ------ //
void ui_start() {
    // Initialize the global app state
    app_state app = {0};
    app_init(&app);

    tb_init();
    int y = 5;
    push_page(MAIN_PAGE, &app);
    tb_shutdown();
}

void app_init(app_state *app) {
    app->channel_list = NULL; 
    app->channel_count = 0;
    app->page_stack = NULL;
    app->current_page_handlers = NULL;
}

void app_destroy(app_state *app) {
    free(app->channel_list);
}

void push_page(page_type page_id, app_state *app) {
    // A page that is pushed, gets immediately rendered.
    page_handlers *current_handlers = app->current_page_handlers;
    page_handlers *handlers = &page_handlers_table[page_id];
    
    if (current_handlers != NULL && current_handlers->destroy != NULL) {
        current_handlers->destroy();
    }
    app->current_page_handlers = handlers;
    handlers->create(app);
}
