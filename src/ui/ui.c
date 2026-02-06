#include "ui.h"
#include "ui_utils.h" 
#include "../list.h"
#include "../logger.h"

#include "pages/handlers.h"

#define TB_IMPL

#include "../termbox2/termbox2.h"
#include <string.h>

static page_handlers page_handlers_table[PAGE_COUNT] = {
    [MAIN_PAGE] = {
        .create = main_menu,
        .destroy = NULL,
    },
    [FEED_PAGE] = {
        .create = main_feed,
        .destroy = main_feed_destroy,
    }, 
    [ARTICLE_PAGE] = {
        .create = article_page,
        .destroy = NULL,
    },
    [CHANNELS_PAGE] = {
        .create = manage_channels_page,
        .destroy = NULL,
    },
    [CHANNEL_PAGE] = {
        .create = channel_page,
        .destroy = NULL,
    },
    [IMPORT_PAGE] = {
        .create = import_page,
        .destroy = NULL,
    },
    [REFRESH_PAGE] = {
        .create = refresh_page,
        .destroy = NULL,
    }
};

// ------ Main UI Call ------ //
void ui_start(initial_state init_state) {
    // Initialize the global app state
    app_state app = {
        .init_state = init_state,
    };
    app_init(&app);

    tb_init();

    navigate(MAIN_PAGE, &app, (local_state){});

    while(app.current_page.type != EXIT_PAGE) {
        log_debug("Clearing screen for current page %i", app.current_page.type);
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
    log_debug("Navigating from: %i to %i", app->current_page.type, page_id);
    // A page that is pushed, gets immediately rendered.
    reset_dividers();
    page previous_page = app->current_page;
    page current_page = {
        .handlers = page_handlers_table[page_id],
        .state = state,
        .type = page_id,
    };

    if (previous_page.handlers.destroy != NULL) {
        previous_page.handlers.destroy();
    }
    app->previous_page = previous_page;
    app->current_page = current_page;
}
