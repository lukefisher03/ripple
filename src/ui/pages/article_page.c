#include "handlers.h"
#include "../ui_utils.h"
#include "../../logger.h"

char *options[] = {
    "Back",
    "Home",
    "Exit",
};

size_t options_length = sizeof(options) / sizeof(options[0]);

void article_page(app_state *app, local_state *state) {
    article_page_state article_state = state->article_state;
    rss_item *article = article_state.item;
    log_debug("Loaded article: %s", article->title);
    int selection = display_basic_menu(10, options, sizeof(char *), options_length);

    switch (selection)
    {
    case 0:
        navigate(FEEDS_PAGE, app, (local_state){});
        break;
    case 1:
        navigate(MAIN_PAGE, app, (local_state){});
        break;
    default:
        navigate(EXIT_PAGE, app, (local_state){});
        break;
    }
}