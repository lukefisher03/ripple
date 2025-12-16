#include "feeds_page.h"
#include "../ui_utils.h"

static int COL_GAP = 2;
static int MIN_WIDTH = 100;
static channel_column_widths COL_WIDTHS;

static int render_feed_article_selections(int x, int y, bool selected, const void *it);

void feed_reader(Channel **channel_list) {
    Channel *c = channel_list[0];
    Arena arena;
    struct tb_event ev;
    int width = tb_width();
    arena_init(&arena, 4096);

    tb_clear();
    int y = 5;
    char *divider = arena_allocate(&arena, tb_width());
    for (size_t i = 0; i < width; i++) {
        divider[i] = '-';
    }
    tb_printf(0, y++, TB_GREEN, 0, "Feed Reader");
    tb_printf(0, y++, TB_GREEN, 0, divider);
    display_menu(y, c->items->elements, sizeof(Item *), c->items->count, &render_feed_article_selections);
    tb_present();
    tb_poll_event(&ev);
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
    Item *item = *(Item**)it;
    Arena arena;
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
    write_column(str + offset, item->pub_date, COL_WIDTHS.pub_date_width);

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