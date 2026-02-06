#include "handlers.h"
#include "../ui_utils.h"
#include "../../utils.h"
#include "../../logger.h"
#include "../../channels_db/channel_db_api.h"

#include <string.h>

static feed_column_widths col_widths = {
    .channel_name = 0.2,
    .title = 0.6,
    .pub_date = 0.2,
};

extern char *thick_divider;
extern char *thin_divider;
extern char *blank_line;

static int render_feed_article_selections(renderer_params *params);

void main_feed(app_state *app, local_state *state){
    (void) state;
    int width = tb_width();

    generic_list *items = list_init();
    if (get_main_feed_articles(items) != 0) {
        log_debug("Could not load channels due to memory error!");       
    } 

    int y = 1;
    
    char *row = malloc(width + 1);
    size_t offset = 0;

    offset += add_column(row + offset, col_widths.channel_name * width, "CHANNEL");
    offset += add_column(row + offset, col_widths.title * width, "TITLE");
    offset += add_column(row + offset, col_widths.pub_date * width, "DATE");
    memset(row + offset, ' ', width - offset);
    row[width] = '\0';


    write_centered(y++, TB_GREEN, 0, "ARTICLE FEED");
    tb_printf(0, y++, TB_GREEN, 0, "%s", row);
    tb_printf(0, y++, TB_GREEN, 0, "%s", thick_divider);

    int nav_help_offset = 3;
    nav_help_offset += print_navigation_help(nav_help_offset, tb_height() - 2, "ENTER", "view article");
    nav_help_offset += print_navigation_help(nav_help_offset, tb_height() - 2, "b", "back");
    nav_help_offset += print_navigation_help(nav_help_offset, tb_height() - 2, "E", "exit");

    menu_config config = {
        .y = y,
        .x = 0,
        .options = items->elements,
        .option_size = sizeof(article_with_channel_name*),
        .option_count = items->count,
        .renderer = &render_feed_article_selections,
        .valid_input_list = "bE",
        .valid_input_count = 2,
        .row = row,
        .row_length = width,
    };

    // This gets overwritten if there's articles to display
    write_centered(y + 2, TB_GREEN, 0, "no articles, start by adding a channel");

    menu_result result = display_menu(config);
    article_with_channel_name *selected_item = NULL;
    int selected_article_id = -1;

    if (!list_is_empty(items)) {
        selected_item = items->elements[result.selection];
        selected_article_id = selected_item->item->id;
    } 

    for (size_t i = 0; i < items->count; i++) {
        free_article_with_channel_name(items->elements[i]);
    }
    list_free(items);

    switch (result.ev.ch) {
        case 'b':
            navigate(MAIN_PAGE, app, (local_state){});
            break;
        case 'E':
            navigate(EXIT_PAGE, app, (local_state){});
            break;
        default:
           break;
    }

    if (result.ev.key == TB_KEY_ENTER && selected_item != NULL) {
        navigate(ARTICLE_PAGE, app, (local_state){
            .page = ARTICLE_PAGE,
            .article_state = {
                .article_id = selected_article_id,
            },
        });
    }
}

static int render_feed_article_selections(renderer_params *params) {
    article_with_channel_name *article = *(article_with_channel_name**)params->option;
    rss_item *item = article->item;
    int new_y = params->start_y;
    
    char *row = params->config->row;
    int row_length = params->config->row_length;


    size_t offset = 0;
    offset += add_column(row + offset, col_widths.channel_name * row_length, article->channel_name);
    offset += add_column(row + offset, col_widths.title * row_length, item->title);
    char formatted_date[128] = "";
    unix_time_to_formatted(item->unix_timestamp, formatted_date, 128);
    offset += add_column(row + offset, col_widths.pub_date * row_length, formatted_date);
    memset(row + offset, ' ', row_length - offset);
    row[row_length] = '\0';

    uintattr_t bg = params->selected ? TB_BLACK : 0;

    tb_printf(0, new_y++, 0, bg, "%s", blank_line);
    tb_printf(0, new_y++, TB_GREEN, bg, "%s", row);   
    tb_printf(0, new_y++, 0, bg, "%s", blank_line);
    tb_printf(0, new_y++, TB_GREEN, 0, "%s", thin_divider);

    return new_y - params->start_y;
}