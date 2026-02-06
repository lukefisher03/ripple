#include "handlers.h"
#include "../../logger.h"
#include "../ui.h"
#include "../ui_utils.h"
#include "../../list.h"
#include "../../termbox2/termbox2.h"
#include "../../utils.h"

channel_page_column_widths  col_widths = {0};
extern char *thick_divider;
extern char *thin_divider;
extern char *blank_line;
// TODO: Update how we do rows
char *row = NULL;

static void set_column_widths(channel_page_column_widths *widths, int screen_width); 
static int render_article_list(renderer_params *params);

void channel_page(app_state *app, local_state *state) {
    int width = tb_width();
    row = calloc(width + 1, sizeof(char));
    set_column_widths(&col_widths, width);
    rss_channel channel = {0};
    if (get_channel(state->channel_state.channel_id, &channel) != 0) {
        log_debug("Failed to get channel!");
    }

    int offset = 0;
    offset += add_column(row + offset, col_widths.title, "TITLE");
    offset += add_column(row + offset, col_widths.author, "AUTHOR");
    offset += add_column(row + offset, col_widths.date, "DATE");
    memset(row + offset, ' ', width - offset);
    row[width] = '\0';

    int y = 1;
    write_centered(y++, TB_GREEN, 0, channel.title);
    y++;
    tb_printf(0, y++, TB_GREEN, 0, "%s", row);
    tb_printf(0, y++, TB_GREEN, 0, "%s", thick_divider);

    generic_list *article_list = list_init();
    get_channel_articles(&channel, article_list);

    int nav_help_offset = 3;
    nav_help_offset += print_navigation_help(nav_help_offset, tb_height() - 2, "b", "BACK");
    nav_help_offset += print_navigation_help(nav_help_offset, tb_height() - 2, "h", "HOME");
    nav_help_offset += print_navigation_help(nav_help_offset, tb_height() - 2, "E", "EXIT");

    menu_config config = {
        .y = y,
        .x = 0,
        .options = article_list->elements,
        .option_size = sizeof(rss_item*),
        .option_count = article_list->count,
        .renderer = &render_article_list,
        .valid_input_list = "hbE",
        .valid_input_count = 3,
    };

    // This gets overwritten if there's articles to display
    write_centered(y + 2, TB_GREEN, 0, "no articles, start by adding a channel");

    menu_result result = display_menu(config);

    rss_item *selected_article = article_list->elements[result.selection];
    int selected_article_id = selected_article->id;

    for (size_t i = 0; i < article_list->count; i++) {
        free_item(article_list->elements[i]);
    }
    list_free(article_list);
    
    switch(result.ev.ch) {
        case 'h':
            navigate(MAIN_PAGE, app, (local_state){});
            break;
        case 'b':
            navigate(CHANNELS_PAGE, app, (local_state){});
            break;
        case 'E':
            navigate(EXIT_PAGE, app, (local_state){});
            break;
        default: {
            navigate(ARTICLE_PAGE, app, (local_state){
                .article_state = {
                    .article_id = selected_article_id,
                }
            });
            break;
        }
    }
    tb_present();
}

static void set_column_widths(channel_page_column_widths *widths, int screen_width) {
    widths->title = screen_width * 0.5;
    widths->author = screen_width * 0.3;
    widths->date = screen_width * 0.2;
}

static int render_article_list(renderer_params *params) {
    int width = tb_width();
    rss_item *article = *(rss_item **)params->option;
    
    int offset = 0;
    offset += add_column(row + offset, col_widths.title, article->title);
    offset += add_column(row + offset, col_widths.author, article->author);
    char formatted_date[256]; // TODO: clean this up
    unix_time_to_formatted(article->unix_timestamp, formatted_date, 256);
    offset += add_column(row + offset, col_widths.date, formatted_date);
    memset(row + offset, ' ',width - offset);
    row[width] = '\0';

    int new_y = params->start_y;
    uintattr_t bg = params->selected ? TB_BLACK : 0;
    tb_printf(0, new_y++, TB_GREEN, bg, "%s", blank_line);
    tb_printf(0, new_y++, TB_GREEN, bg, "%s", row);
    tb_printf(0, new_y++, TB_GREEN, bg, "%s", blank_line);
    tb_printf(0, new_y++, TB_GREEN, 0, "%s", thin_divider);

    return new_y - params->start_y;
}