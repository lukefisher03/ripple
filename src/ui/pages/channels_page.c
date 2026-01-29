// TODO: This should just be the "feed" page. There will be a distinc channels page.
#include "handlers.h"
#include "../../arena.h"
#include "../ui_utils.h"
#include "../../utils.h"
#include "../../logger.h"
#include "../../channels_db/channel_db_api.h"

#include <string.h>
#include <assert.h>

static int MIN_WIDTH = 100;
static channel_column_widths COL_WIDTHS;
static int COL_GAP = 3;

// ------ Forward declarations ------ //
void set_channel_column_widths(channel_column_widths *widths, size_t width);
static int render_channel_article_selections(int x, int y, bool selected, const void *it);
size_t add_column(char *row, int col_width, const char *col_str);
// static void write_column(char *dest, const char *src, size_t max_width);

static char *row; 
static char *blank_line;
static char *thick_divider;
static char *divider;
static size_t SCREEN_WIDTH;

void channel_reader_init(void) {
    SCREEN_WIDTH = tb_width() > MIN_WIDTH ? tb_width() : MIN_WIDTH;
    set_channel_column_widths(&COL_WIDTHS, SCREEN_WIDTH);

    row = malloc(SCREEN_WIDTH + 1);
    blank_line = malloc(SCREEN_WIDTH + 1);
    divider = malloc(SCREEN_WIDTH + 1);
    thick_divider = malloc(SCREEN_WIDTH + 1);
    
    // Should probably handle this better
    if (!blank_line || !divider || !thick_divider) {
        free(blank_line);
        free(divider);
        free(thick_divider);
        log_debug("Out of memory! Could not initialize the channel reader!");
        abort();
    }

    memset(blank_line, ' ', SCREEN_WIDTH);
    memset(divider, '-', SCREEN_WIDTH);
    memset(thick_divider, '=', SCREEN_WIDTH);

    divider[SCREEN_WIDTH] = '\0';
    blank_line[SCREEN_WIDTH] = '\0';
    thick_divider[SCREEN_WIDTH] = '\0';
}

void channel_reader_destroy(void) {
    free(blank_line);
    blank_line = NULL;
    free(divider);
    divider = NULL;
    free(thick_divider);
    thick_divider = NULL;
    free(row);
    row = NULL;
}

void channel_reader(app_state *app, local_state *state){
    (void) state;

    channel_reader_init();

    generic_list *items = list_init();
    if (get_main_feed_articles(items) != 0) {
        log_debug("Could not load channels due to memory error!");       
    } 

    int y = 1;
    
    char *row = malloc(SCREEN_WIDTH + 1);
    size_t offset = 0;

    offset += add_column(row + offset, COL_WIDTHS.channel_name, "CHANNEL");
    offset += add_column(row + offset, COL_WIDTHS.title, "TITLE");
    offset += add_column(row + offset, COL_WIDTHS.pub_date, "DATE");
    memset(row + offset, ' ', SCREEN_WIDTH - offset);
    row[SCREEN_WIDTH] = '\0';


    write_centered(y++, TB_GREEN, 0, "ARTICLE FEED");
    tb_printf(0, y++, TB_GREEN, 0, row);
    tb_printf(0, y++, TB_GREEN, 0, thick_divider);
    free(row);

    int selection = display_menu(y, items->elements, sizeof(article_with_channel_name *), items->count, &render_channel_article_selections);
    article_with_channel_name *selected_item = items->elements[selection];

    list_free(items);

    navigate(ARTICLE_PAGE, app, (local_state){
        .page = ARTICLE_PAGE,
        .article_state = {
            .article = selected_item,
        },
    });
}

int compare_item_timestamps(const void *it1, const void *it2) {
    // Performs comparison in reverse to make the latest articles
    // appear first.
    rss_item *item1 = *(rss_item **)it1;
    rss_item *item2 = *(rss_item **)it2;

    // Start by comparing unix timestamps
    if (item1->unix_timestamp > item2->unix_timestamp)
        return -1;
    if (item1->unix_timestamp < item2->unix_timestamp)
        return 1;

    return -strcmp(item1->title, item2->title);
}

static int render_channel_article_selections(int x, int y, bool selected, const void *it) {
    article_with_channel_name *article = *(article_with_channel_name**)it;
    rss_item *item = article->item;
    int new_y = y;

    set_channel_column_widths(&COL_WIDTHS, SCREEN_WIDTH);

    size_t offset = 0;

    offset += add_column(row + offset, COL_WIDTHS.channel_name, article->channel_name);
    offset += add_column(row + offset, COL_WIDTHS.title, item->title);
    char formatted_date[128] = "";
    unix_time_to_formatted(item->unix_timestamp, formatted_date, 128);
    offset += add_column(row + offset, COL_WIDTHS.pub_date, formatted_date);
    memset(row + offset, ' ', SCREEN_WIDTH - offset);
    row[SCREEN_WIDTH] = '\0';

    uintattr_t bg = selected ? TB_BLACK : 0;

    tb_printf(0, new_y++, 0, bg, blank_line);
    tb_printf(0, new_y++, TB_GREEN, bg, row);   
    tb_printf(0, new_y++, 0, bg, blank_line);
    tb_printf(0, new_y++, TB_GREEN, 0, divider);

    return new_y - y;
}

size_t add_column(char *row, int col_width, const char *col_str) {
    assert(row != NULL);
    log_debug("Column width: %i", col_width);
    assert(col_width >= COL_GAP + 3);

    const char *src = col_str ? col_str : "None"; 

    size_t whitespace_count = 0;
    for (; src[whitespace_count] != '\0' && src[whitespace_count] == ' '; whitespace_count++);
    size_t col_str_len = strlen(src) - whitespace_count;
    
    memset(row, ' ', col_width);

    if (col_str_len <= col_width - COL_GAP) {
        memcpy(row, src + whitespace_count, col_str_len);
    } else {
        memcpy(row, src + whitespace_count, col_width - COL_GAP - 3);
        memset(row + col_width - COL_GAP - 3, '.', 3);
    } 

    return col_width;
}

void set_channel_column_widths(channel_column_widths *widths, size_t width) {
    widths->channel_name = (int)(0.2 * width);
    widths->title = (int)(0.6 * width);
    widths->pub_date = (int)(0.2 * width);
}