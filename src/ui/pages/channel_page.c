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
static int render_article_list(int x, int y, bool selected, const void *article);

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
    tb_printf(0, y++, TB_GREEN, 0, row);
    tb_printf(0, y++, TB_GREEN, 0, thick_divider);

    generic_list *article_list = list_init();
    get_channel_articles(&channel, article_list);

    menu_result result = display_menu(y, article_list->elements, sizeof(rss_item *), article_list->count, &render_article_list);

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

static int render_article_list(int x, int y, bool selected, const void *cur_article) {
    int width = tb_width();
    rss_item *article = *(rss_item **)cur_article;
    
    int offset = 0;
    offset += add_column(row + offset, col_widths.title, article->title);
    offset += add_column(row + offset, col_widths.author, article->author);
    char formatted_date[256]; // TODO: clean this up
    unix_time_to_formatted(article->unix_timestamp, formatted_date, 256);
    offset += add_column(row + offset, col_widths.date, formatted_date);
    memset(row + offset, ' ',width - offset);
    row[width] = '\0';

    int new_y = y;
    uintattr_t bg = selected ? TB_BLACK : 0;
    tb_printf(0, new_y++, TB_GREEN, bg, blank_line);
    tb_printf(0, new_y++, TB_GREEN, bg, row);
    tb_printf(0, new_y++, TB_GREEN, bg, blank_line);
    tb_printf(0, new_y++, TB_GREEN, 0, thin_divider);

    return new_y - y;
}