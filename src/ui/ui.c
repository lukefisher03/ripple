// Prevent redefinitions by putting this before
// the definition of TB_IMPL
#include "ui.h"
#include "ui_utils.h" 
#include "../list.h"
#include "../logger.h"

#include "pages/handlers.h"

#define TB_IMPL

// Now after TB_IMPL is defined, the definitions will be included here.
#include "../termbox2/termbox2.h"
#include "../arena.h"

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
    navigate(MAIN_PAGE, &app, (local_state){});

    while(app.current_page.type != EXIT_PAGE) {
        tb_clear();
        local_state *st = &app.current_page.state;
        page_create create = app.current_page.handlers.create;
        create(&app, st);
    }
    tb_shutdown();
}

void app_init(app_state *app) {
    app->channel_list = NULL; 
    app->channel_count = 0;
}

void app_destroy(app_state *app) {
    free(app->channel_list);
}

void navigate(page_type page_id, app_state *app, local_state state) {
    // A page that is pushed, gets immediately rendered.
    page previous_page = app->current_page;
    page current_page = {
        .handlers = page_handlers_table[page_id],
        .state = state,
        .type = page_id,
    };

    if (previous_page.handlers.destroy != NULL) {
        previous_page.handlers.destroy();
    }

    app->current_page = current_page;
}
