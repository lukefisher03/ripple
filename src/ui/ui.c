
// Prevent redefinitions by putting this before
// the definition of TB_IMPL
#include "ui.h"
#include "ui_utils.h" 
#include "../list.h"

#define TB_IMPL

// Now after TB_IMPL is defined, the definitions will be included here.
#include "../termbox2/termbox2.h"
#include "../arena.h"

#include <stdarg.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>
#include <string.h>

static page_renderer page_table[PAGE_COUNT] = {
    [MAIN_PAGE] = main_menu,
    [FEEDS_PAGE] = feed_reader,
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
}

void app_destroy(app_state *app) {
    free(app->channel_list);
}

void push_page(page_type page_id, app_state *app) {
    // A page that is pushed, gets immediately rendered.
    page_table[page_id](app);
}

void pop_page(app_state *app) {
    // unimplemented for now.
    return;
}