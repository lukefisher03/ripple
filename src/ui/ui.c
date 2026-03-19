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
        .destroy = NULL,
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
    },
    [FEEDBACK_PAGE] = {
        .create = feedback_page,
        .destroy = NULL,
    }
};

// ------ Main UI Call ------ //
void ui_start(initial_state init_state) {
    tb_init();
    // Initialize the global app state
    app_state app = {
        .init_state = init_state,
    };

    app.page_stack = bounded_list_init(PAGE_COUNT);

    navigate(MAIN_PAGE, &app, (local_state){});
    
    while(app.current_page->type != EXIT_PAGE) {
        tb_clear();
        local_state *st = &app.current_page->state;
        page_create create = page_handlers_table[app.current_page->type].create;
        create(&app, st);
    }

    for (size_t i = 0; i < app.page_stack->count; i++) {
        free(app.page_stack->elements[i]);
    }
    bounded_list_free(app.page_stack);
    tb_shutdown();
}

void navigate(page_type page_id, app_state *app, local_state state) {
    // A page that is pushed, gets immediately rendered.
    reset_dividers();

    page_destroy destroy = page_handlers_table[app->current_page->type].destroy;
    if (destroy) destroy();

    page *current_page = malloc(sizeof(*current_page));
    current_page->state = state;
    current_page->type = page_id;

    bounded_list_append(app->page_stack, current_page);

    app->current_page = current_page;
}

void navigate_back(app_state *app) {
    if (bounded_list_empty(app->page_stack) || app->page_stack->count == 1) {
        navigate(MAIN_PAGE, app, (local_state){});
        return;
    }

    page *cur_page = bounded_list_pop(app->page_stack);
    page_destroy destroy = page_handlers_table[cur_page->type].destroy;
    if (destroy) destroy();
    free(cur_page);

    app->current_page = bounded_list_peek(app->page_stack);
}