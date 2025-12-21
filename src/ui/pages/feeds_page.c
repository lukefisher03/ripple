#include "feeds_page.h"
#include "../../arena.h"
#include "../ui_utils.h"
#include "../ui.h"
#include "../../utils.h"
#include "../../logger.h"

#include <string.h>

static int COL_GAP = 2;
static int MIN_WIDTH = 100;
static channel_column_widths COL_WIDTHS;

// ------ Forward declarations ------ //
static int render_feed_article_selections(int x, int y, bool selected, const void *it);
static bool collect_items(rss_channel **channels, size_t channel_count, generic_list *items);
static void write_column(char *dest, const char *src, size_t max_width);

static char *files[] = {
    "test/stack_overflow.xml",
    "test/smart_less.xml",
    "test/ben_hoyt.xml"
};

static char *blank_line;
static char *thick_divider;
static char *divider;
static int SCREEN_WIDTH;

void feed_reader_init(void) {
    SCREEN_WIDTH = tb_width();
    tb_clear();
    set_feed_column_widths();

    blank_line = malloc(SCREEN_WIDTH+ 1);
    divider = malloc(SCREEN_WIDTH + 1);
    thick_divider = malloc(SCREEN_WIDTH + 1);

    memset(blank_line, ' ', SCREEN_WIDTH);
    memset(divider, '-', SCREEN_WIDTH);
    memset(thick_divider, '=', SCREEN_WIDTH);

    divider[SCREEN_WIDTH] = '\0';
    blank_line[SCREEN_WIDTH] = '\0';
    thick_divider[SCREEN_WIDTH] = '\0';

}

void feed_reader_destroy(void) {
    free(blank_line);
    free(divider);
    free(thick_divider);
}

void feed_reader(app_state *app){
    feed_reader_init();
    app->channel_count = sizeof(files) / sizeof(files[0]);
    app->channel_list = load_channels(files, app->channel_count);

    generic_list *items = list_init();
    if (!collect_items(app->channel_list, app->channel_count, items)) {
        log_debug("Could not load feeds due to memory error!");       
    } 

    int y = 1;

    size_t offset = 4;
    char *header = malloc(SCREEN_WIDTH + 1);
    memset(header, ' ', SCREEN_WIDTH);
    header[SCREEN_WIDTH] = '\0';
    // Should probably refactor how I do columns
    write_column(header + offset, "CHANNEL", COL_WIDTHS.channel_name);
    offset += COL_WIDTHS.channel_name;
    write_column(header + offset, "ARTICLE", COL_WIDTHS.title);
    offset += COL_WIDTHS.title;
    write_column(header + offset, "AUTHOR", COL_WIDTHS.author);
    offset += COL_WIDTHS.author;
    write_column(header + offset, "DATE", COL_WIDTHS.pub_date);
    
    tb_printf(0, y++, TB_GREEN, 0, "FEED READER");
    tb_printf(0, y++, TB_GREEN, 0, header);
    tb_printf(0, y++, TB_GREEN, 0, thick_divider);
    free(header);
    display_menu(y, items->elements, sizeof(rss_item *), items->count, &render_feed_article_selections);
    feed_reader_destroy();
    list_free(items);

    push_page(MAIN_PAGE, app);
}

int compare_item_timestamps(const void *it1, const void *it2) {
    rss_item *item1 = *(rss_item **)it1;
    rss_item *item2 = *(rss_item **)it2;

    // Start by comparing unix timestamps
    if (item1->pub_date_unix > item2->pub_date_unix)
        return -1;
    if (item1->pub_date_unix < item2->pub_date_unix)
        return 1;

    return -strcmp(item1->title, item2->title);
}

static bool collect_items(rss_channel **channels, size_t channel_count, generic_list *items) {
    // Combine all the channel's various articles
    for (size_t i = 0; i < channel_count; i++) {
        for (size_t j = 0; j < channels[i]->items->count; j++) {
            rss_item *it = channels[i]->items->elements[j];
            if (!list_append(items, it)) {
                return false;
            }
        }
    }
    qsort(items->elements, items->count, sizeof(rss_item *), compare_item_timestamps); 
    return true;
}

static void write_column(char *dest, const char *src, size_t max_width) {
    if (src) {
        size_t src_len = strlen(src);
        size_t final_width = src_len;
        if (final_width > max_width) {
            final_width = max_width;
        }
        
        for (size_t i = 0; i < final_width; i++) {
            // These columns should only be 1 line, so remove any newlines 
            // and replace with spaces.
            if (src[i] != '\n') {
                dest[i] = src[i];
            } else {
                dest[i] = ' ';
            }
        }
        
        if (final_width >= max_width - COL_GAP) {
            memcpy(dest + max_width - 3 - COL_GAP, "...", 3);
            memset(dest + max_width - COL_GAP, ' ', max_width - COL_GAP);
        }

    } else {
        memcpy(dest, "None", 4);
    }
}

static int render_feed_article_selections(int x, int y, bool selected, const void *it) {
    rss_item *item = *(rss_item**)it;
    
    int width = SCREEN_WIDTH > MIN_WIDTH ? SCREEN_WIDTH : MIN_WIDTH;
    int new_y = y;

    char *str = malloc(width + 1); 
    memset(str, ' ', width); // Fill the entire buffer with empty space

    size_t l;
    size_t offset = 4; // Width of the selector icon on the left
    write_column(str + offset, item->channel->title, COL_WIDTHS.channel_name);
    offset += COL_WIDTHS.channel_name;
    write_column(str + offset, item->title, COL_WIDTHS.title);
    offset += COL_WIDTHS.title;
    write_column(str + offset, item->author, COL_WIDTHS.author);
    offset += COL_WIDTHS.author;
    write_column(str + offset, item->pub_date_string, COL_WIDTHS.pub_date);

    uintattr_t bg = 0;
    if (selected) {
        memcpy(str, "-> ", 3);
        bg = TB_BLACK;
    }
    memset(str + width, '\0', 1);

    tb_printf(0, new_y++, 0, bg, blank_line);

    int err;
    if ((err = tb_printf(0, new_y++, TB_GREEN, bg, str)) != 0) {
        log_debug("Error printing to screen with termbox! %i", err);
    }
    tb_printf(0, new_y++, 0, bg, blank_line);
    tb_printf(0, new_y++, TB_GREEN, 0, divider);
    free(str);

    return new_y - y;
}

void set_feed_column_widths(void) {
    int width = SCREEN_WIDTH > MIN_WIDTH ? SCREEN_WIDTH : MIN_WIDTH;
    COL_WIDTHS.channel_name = (int)(0.2 * width);
    COL_WIDTHS.title = (int)(0.5 * width);
    COL_WIDTHS.author = (int)(0.2 * width);
    COL_WIDTHS.pub_date = (int)(0.1 * width);
}