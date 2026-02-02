#include "handlers.h"
#include "../ui_utils.h"
#include "../../logger.h"
#include "../../utils.h"

char *article_options[] = {
    "back",
    "home",
    "exit",
};

size_t PADDING = 10;
size_t options_length = sizeof(article_options) / sizeof(article_options[0]);

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

    // TODO: Cleanup this part and the state transition stuff
    article_page_state article_state = state->article_state;
    rss_item *item = article_state.article->item;
    char *channel_title = article_state.article->channel_name;

    log_debug("Loaded article: %s", item->title);

    int y = 5;
    tb_printf(PADDING, y++, TB_GREEN, 0, item->title);
    tb_printf(PADDING, y++, TB_GREEN, 0, divider);
    tb_printf(PADDING, y++, TB_GREEN, 0, channel_title);
    tb_printf(PADDING, y++, TB_GREEN, 0, item->link);
    char formatted_time[128];
    unix_time_to_formatted(item->unix_timestamp, formatted_time, 128);
    tb_printf(PADDING, y++, TB_GREEN, 0, formatted_time);
    y += 5;

    size_t description_length = strlen(item->description);
    size_t lines = 0;
    size_t d_len = 0;

    char *description = malloc(description_length + height);

    for (size_t i = 0; i <= description_length; i++) {
        char ch = item->description[i];
        if (ch != '\n') {
            description[d_len++] = item->description[i];
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
    menu_result result = display_basic_menu(y++, article_options, options_length);

    free(divider);
    free(description);

    switch (result.selection) {
        case 0:
            navigate(FEED_PAGE, app, (local_state){});
            break;
        case 1:
            navigate(MAIN_PAGE, app, (local_state){});
            break;
        default:
            navigate(EXIT_PAGE, app, (local_state){});
            break;
    }
}