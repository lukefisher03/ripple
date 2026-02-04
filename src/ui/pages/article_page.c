#include "handlers.h"
#include "../ui_utils.h"
#include "../../logger.h"
#include "../../utils.h"

char *article_options[] = {
    "back",
    "home",
    "exit",
};

extern char *thin_divider;

size_t PADDING = 10;
size_t options_length = sizeof(article_options) / sizeof(article_options[0]);

char *format_description(char *description, int width, int *lines);

void article_page(app_state *app, local_state *state) {
    int width = tb_width();

    if (width > PADDING * 2) {
        width -= PADDING * 2;
    } else {
        PADDING = 0;
    }

    char *divider = calloc(width + 1, sizeof(char));
    memcpy(divider, thin_divider, width);
    divider[width] = '\0';

    // TODO: Cleanup this part and the state transition stuff
    article_page_state article_state = state->article_state;
    rss_item item = {0};
    if (get_article(article_state.article_id, &item) != 0) {
        navigate(MAIN_PAGE, app, (local_state){});
    }

    rss_channel chan = {0};
    get_channel(item.channel_id, &chan);

    int y = 5;
    tb_printf(PADDING, y++, TB_GREEN, 0, item.title);
    tb_printf(PADDING, y++, TB_GREEN, 0, divider);
    tb_printf(PADDING, y++, TB_GREEN, 0, chan.title);
    tb_printf(PADDING, y++, TB_GREEN, 0, item.link);
    char formatted_time[128];
    unix_time_to_formatted(item.unix_timestamp, formatted_time, 128);
    tb_printf(PADDING, y++, TB_GREEN, 0, formatted_time);
    y += 5;
    int lines = 0;
    char *description = format_description(item.description, width, &lines);

    tb_printf(PADDING, y++, TB_GREEN, 0, "DESCRIPTION");
    tb_printf(PADDING, y++, TB_GREEN, 0, description ? description : "No description provided");
    y += lines + 5;
    menu_result result = display_basic_menu(y++, article_options, options_length);

    free(divider);
    free(description);

    switch (result.selection) {
        case 0:
            navigate(app->previous_page.type, app, app->previous_page.state);
            break;
        case 1:
            navigate(MAIN_PAGE, app, (local_state){});
            break;
        default:
            navigate(EXIT_PAGE, app, (local_state){});
            break;
    }
}

char *format_description(char *description, int width, int *lines) {
    int height = tb_height();
    if (!description) {
        return NULL;
    }

    size_t description_length = strlen(description);
    *lines = 0;
    size_t d_len = 0;

    char *formatted_description = malloc(description_length + height);

    for (size_t i = 0; i <= description_length; i++) {
        char ch = description[i];
        if (ch != '\n') {
            formatted_description[d_len++] = description[i];
        }
        if (d_len % width == 0) {
            formatted_description[d_len++] = '\n';
            *lines += 1;
        }
    }
    description[d_len] = '\0';

    return formatted_description;
}