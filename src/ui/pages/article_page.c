#include "handlers.h"
#include "../ui_utils.h"
#include "../../logger.h"

char *options[] = {
    "Back",
    "Home",
    "Exit",
};

size_t PADDING = 10;
size_t options_length = sizeof(options) / sizeof(options[0]);

void article_page(app_state *app, local_state *state) {
    int width = tb_width();

    if (width > PADDING * 2) {
        width -= PADDING * 2;
    } else {
        PADDING = 0;
    }

    char *divider = malloc(width + 1);
    memset(divider, '-', width);
    divider[width] = '\0';

    int height = tb_height();

    article_page_state article_state = state->article_state;
    rss_item *article = article_state.item;

    log_debug("Loaded article: %s", article->title);

    int y = 5;
    tb_printf(PADDING, y++, TB_GREEN, 0, article->title);
    tb_printf(PADDING, y++, TB_GREEN, 0, divider);
    tb_printf(PADDING, y++, TB_GREEN, 0, article->channel->title);
    tb_printf(PADDING, y++, TB_GREEN, 0, article->link);
    y += 5;

    size_t description_length = strlen(article->description);
    size_t lines = 0;
    size_t d_len = 0;

    char *description = malloc(description_length + height);

    for (size_t i = 0; i <= description_length; i++) {
        char ch = article->description[i];
        if (ch != '\n') {
            description[d_len++] = article->description[i];
        }
        if (d_len % width == 0) {
            description[d_len++] = '\n';
            lines += 1;
        }
    }
    description[d_len] = '\0';

    tb_printf(PADDING, y++, TB_GREEN, 0, "DESCRIPTION");
    tb_printf(PADDING, y++, TB_GREEN, 0, description);
    y += lines + 5;
    int selection = display_basic_menu(y++, options, sizeof(char *), options_length);

    free(divider);
    free(description);

    switch (selection) {
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