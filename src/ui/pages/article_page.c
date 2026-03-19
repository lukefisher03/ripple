#include "handlers.h"
#include "../ui_utils.h"
#include "../../logger.h"
#include "../../utils.h"

#define MAX_ARTICLE_DESCRIPTION 1024

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
    rss_item *item = item_init();
    if (get_article(article_state.article_id, item) != 0) {
        navigate(MAIN_PAGE, app, (local_state){});
    }

    rss_channel *chan = channel_init(); 
    get_channel(item->channel_id, chan);

    int y = 5;
    tb_printf(PADDING, y++, TB_GREEN, 0, "%s", item->title);
    tb_printf(PADDING, y++, TB_GREEN, 0, "%s", divider);
    tb_printf(PADDING, y++, TB_GREEN, 0, "%s", chan->title);
    tb_printf(PADDING, y++, TB_WHITE, TB_GREEN, "%s", item->link);
    char formatted_time[128];
    unix_time_to_formatted(item->unix_timestamp, formatted_time, 128);
    tb_printf(PADDING, y++, TB_GREEN, 0, "%s", formatted_time);
    y += 5;
    int lines = 0;
    char *description = format_description(item->description, width, &lines);

    tb_printf(PADDING, y++, TB_GREEN, 0, "DESCRIPTION");
    errno = 0;
    tb_printf(PADDING, y++, TB_GREEN, 0, "%s", description != NULL ? description : "No description provided");
    y += lines + 5;
    menu_result result = display_basic_menu(tb_height() - 10, article_options, options_length);

    free_item(item);
    free_channel(chan);
    free(divider);
    free(description);

    switch (result.selection) {
        case 0:
            navigate_back(app);
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
    size_t i = 0;
    for (; i < description_length && (description[i] == ' ' || description[i] == '\n' || description[i] == '\t'); i++) {
        description_length--; 
    }

    int description_overflow = description_length > MAX_ARTICLE_DESCRIPTION - 3;
    size_t final_length = description_overflow ? MAX_ARTICLE_DESCRIPTION : description_length;

    *lines = 0;
    size_t d_len = 0;

    char *formatted_description = malloc(final_length + height);
    for (; i < final_length; i++) {
        char ch = description[i];
        if (ch != '\n') {
            formatted_description[d_len++] = description[i];
        }
        if (d_len % width == 0) {
            formatted_description[d_len++] = '\n';
            *lines += 1;
        }
    }

    if (description_overflow) {
        memset(formatted_description + d_len - 3, '.', 3);
    }

    if (d_len == 0) return NULL;

    formatted_description[d_len] = '\0';

    return formatted_description;
}