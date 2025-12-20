#include "feeds_page.h"
#include "../../arena.h"
#include "../ui_utils.h"
#include "../ui.h"
#include "../../utils.h"
#include "../../logger.h"

static int COL_GAP = 2;
static int MIN_WIDTH = 100;
static channel_column_widths COL_WIDTHS;

// ------ Forward declarations ------ //
static int render_feed_article_selections(int x, int y, bool selected, const void *it);
static bool collect_items(rss_channel **channels, size_t channel_count, generic_list *items);

static char *files[] = {
    "test/stack_overflow.xml",
    "test/smart_less.xml",
};

void feed_reader(app_state *app){
    tb_clear();

    app->channel_count = sizeof(files) / sizeof(files[0]);
    app->channel_list = load_channels(files, app->channel_count);

    generic_list *items = list_init();
    if (!collect_items(app->channel_list, app->channel_count, items)) {
        log_debug("Could not load feeds due to memory error!");       
    } 

    mem_arena arena;
    arena_init(&arena, 4096);

    int width = tb_width();
    set_feed_column_widths();
    int y = 1;
    char *divider = arena_allocate(&arena, tb_width());
    for (size_t i = 0; i < width; i++) {
        divider[i] = '-';
    }

    tb_printf(0, y++, TB_GREEN, 0, "Feed Reader");
    tb_printf(0, y++, TB_GREEN, 0, divider);
    display_menu(y, items->elements, sizeof(rss_item *), items->count, &render_feed_article_selections);
    push_page(MAIN_PAGE, app);

    arena_free(&arena);
    list_free(items);
}

int compare_item_timestamps(const void *it1, const void *it2) {
    // it1 and it2 are pointers to pointers to rss_item hence the dereference
    // here. When the comparison function is called by qsort, it passes in each
    // array element by pointer which creates the double indirection.
    rss_item *item1 = *(rss_item **)it1;
    rss_item *item2 = *(rss_item **)it2;

    // Start by comparing unix timestamps
    if (item1->pub_date_unix > item2->pub_date_unix)
        return -1;
    if (item1->pub_date_unix < item2->pub_date_unix)
        return 1;
    // If the timestamps are equal, compare their titles alphabetically
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
            // log_debug("Collected item: %s", it->title);
        }
    }
    
    // log_debug("Finished collecting items, total: %lu", items->count);
    qsort(items->elements, items->count, sizeof(rss_item *), compare_item_timestamps); 
    log_debug("Finished sorting articles by timestamp");
    for (size_t i = 0; i < items->count; i++) {
        log_debug("Showing item: %s", ((rss_item *)items->elements[i])->title);
    }
   return true;
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
    int err;
    if ((err = tb_printf(0, new_y++, TB_GREEN, 0, str)) != 0) {
        log_debug("Error printing to screen with termbox! %i", err);
    }
    new_y++;
    tb_printf(0, new_y++, TB_GREEN, 0, divider);

    arena_free(&arena);
    return new_y - y;
}

void set_feed_column_widths(void) {
    int width = tb_width() > MIN_WIDTH ? tb_width() : MIN_WIDTH;

    COL_WIDTHS.title_width = (int)(0.5 * width);
    COL_WIDTHS.author_width = (int)(0.3 * width);
    COL_WIDTHS.pub_date_width = (int)(0.2 * width);
}