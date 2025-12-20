#include "feeds_page.h"
#include "../../arena.h"
#include "../ui_utils.h"
#include "../ui.h"
#include "../../utils.h"

static int COL_GAP = 2;
static int MIN_WIDTH = 100;
static channel_column_widths COL_WIDTHS;

static int render_feed_article_selections(int x, int y, bool selected, const void *it);
static char *files[] = {
    "test/smart_less.xml",
    "test/stack_overflow.xml",
};

void feed_reader(app_state *app){
    app->channel_list = load_channels(files, 2);
    app->channel_count = 2;

    generic_list *item_list = list_init();
    
    // Combine all the channel's various articles
    for (size_t i = 0; i < app->channel_count; i++) {
        for (size_t j = 0; j < app->channel_list[i]->items->count; j++) {
            rss_item *it = app->channel_list[i]->items->elements[j];
            list_append(item_list, it);
        }
    }

    mem_arena arena;
    struct tb_event ev;
    int width = tb_width();

    arena_init(&arena, 4096);

    set_feed_column_widths();
    tb_clear();
    int y = 1;
    char *divider = arena_allocate(&arena, tb_width());
    for (size_t i = 0; i < width; i++) {
        divider[i] = '-';
    }
    tb_printf(0, y++, TB_GREEN, 0, "Feed Reader");
    tb_printf(0, y++, TB_GREEN, 0, divider);
    display_menu(y, item_list->elements, sizeof(rss_item *), item_list->count, &render_feed_article_selections);
    push_page(MAIN_PAGE, app);
    arena_free(&arena);
}

static void write_column(char *dest, char *src, size_t max_width) {
    if (src) {
        size_t l = strlen(src);
        char *end = memcpy(dest, src, l < max_width ? l : max_width);
        
        if (l >= max_width - COL_GAP) {
            memcpy(dest + max_width - 3 - COL_GAP, "...", 3);
            memset(dest + max_width - COL_GAP, ' ', max_width - COL_GAP);
        }

    } else {
        memcpy(dest, "None", 4);
    }
}

static int render_feed_article_selections(int x, int y, bool selected, const void *it) {
    rss_item *item = *(rss_item**)it;
    
    mem_arena arena;
    int width = tb_width() > MIN_WIDTH ? tb_width() : MIN_WIDTH;
    int new_y = y;

    arena_init(&arena, 4096);

    char *divider = arena_allocate(&arena, width+1);
    memset(divider, '-', width);
    memset(divider + width, '\0', 1);

    char *str = arena_allocate(&arena, width+1);
    memset(str, ' ', width); // Fill the entire buffer with empty space

    size_t l;
    size_t offset = 3; // Width of the selector icon on the left
    write_column(str + offset, item->title, COL_WIDTHS.title_width);
    offset += COL_WIDTHS.title_width;
    write_column(str + offset, item->author, COL_WIDTHS.author_width);
    offset += COL_WIDTHS.author_width;
    write_column(str + offset, item->pub_date_string, COL_WIDTHS.pub_date_width);

    memset(str + width, '\0', 1);

    if (selected) {
        memcpy(str, "-> ", 3);
    }
    new_y++; // Add extra space
    tb_printf(0, new_y++, TB_GREEN, 0, str);
    new_y++;
    tb_printf(0, new_y++, TB_GREEN, 0, divider);

    arena_free(&arena);
    return new_y - y;
}

void set_feed_column_widths(void) {
    int width = tb_width() > MIN_WIDTH ? tb_width() : MIN_WIDTH;

    COL_WIDTHS.title_width = (int)(0.6 * width);
    COL_WIDTHS.author_width = (int)(0.2 * width);
    COL_WIDTHS.pub_date_width = (int)(0.2 * width);
}