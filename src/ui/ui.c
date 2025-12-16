#define TB_IMPL

#include "ui_utils.h"
#include "ui.h"
#include "../arena.h"

#include <stdarg.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>
#include <string.h>


// ------ Globals ------ //

static int MIN_WIDTH = 100;
static channel_column_widths COL_WIDTHS;
static int COL_GAP = 2;

// ------ Forward declarations ------ //
static int main_menu(void);
static void feed_reader(Channel **channel_list);

static int write_centered(int y, uintattr_t fg, uintattr_t bg, const char *text);
static int display_logo(int x, int y, uintattr_t fg, uintattr_t bg);
static int render_main_menu_selection(int x, int y, bool selected, const void *);
static int render_feed_article_selections(int x, int y, bool selected, const void *);
static void set_feed_column_widths(channel_column_widths *widths);

// ------ Main UI Call ------ //
void ui_start(Channel **channel_list, const size_t channel_count) {
    tb_init();
    int y = 5;

    // Calculate how wide the feed columns need to be
    set_feed_column_widths(&COL_WIDTHS);
    // Display the main menu
    int selection = main_menu();

    // Select Menu Option
    switch (selection) {
        case OPTION_FEEDS:
            feed_reader(channel_list);
            break;
        case OPTION_PREFERENCES:
            break;
        case OPTION_FEEDBACK:
            break;
        case OPTION_EXIT:
            break;
        default:
            fprintf(stderr, "INVALID MAIN MENU SELECTION!\n");
            break;
    }

    tb_shutdown();
}

static int main_menu(void) {
    tb_clear();
    int y = 5;
    y += display_logo((tb_width() / 2) - 14, y, TB_WHITE, TB_BLACK);
    y += 2;
    const char *options[] = {
        "Feeds",
        "Preferences",
        "Feedback",
        "Exit",
    };

    write_centered(y + 8, TB_GREEN, 0, "Made by Luke Fisher");

    int ret = display_menu(y, options, sizeof(char *), 4, &render_main_menu_selection);
    tb_present();
    return ret;
}

static void feed_reader(Channel **channel_list) {
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


static int write_centered(int y, uintattr_t fg, uintattr_t bg, const char *text) {
    size_t text_len = strlen(text);
    int mid = tb_width() / 2;
    int v = tb_printf((mid - text_len / 2), y, fg, bg, text);
    return v;
}

static int render_main_menu_selection(int x, int y, bool selected, const void *txt) {
    (void) x; // x is not used for this selection renderer
    char *text = *(char **)txt;
    // There's potentially a better way to do this, but this works for now
    int new_y = y;
    char *selected_s = malloc(strlen(text) + 4); 

    if (!selected_s) {
        fprintf(stderr, "Memory allocation failed for rendering selection.\n");
        return 0;
    } 

    if (selected) {
        sprintf(selected_s, "[ %s ]", text);
    } else {
        sprintf(selected_s, "  %s  ", text);
    }
    write_centered(new_y++, TB_GREEN, 0, selected_s);
    free(selected_s);
    return new_y - y;
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

static int display_logo(int x, int y, uintattr_t fg, uintattr_t bg) {
    int new_y = y;

    tb_printf(x, new_y++, fg, bg, "    ___  _           __    "); 
    tb_printf(x, new_y++, fg, bg, "   / _ \\(_)__  ___  / /__  ");
    tb_printf(x, new_y++, fg, bg, "  / , _/ / _ \\/ _ \\/ / -_) ");
    tb_printf(x, new_y++, fg, bg, " /_/|_/_/ .__/ .__/_/\\__/  ");
    tb_printf(x, new_y++, fg, bg, "       /_/  /_/            ");

    return new_y - y;
}

static void set_feed_column_widths(channel_column_widths *widths) {
    int width = tb_width() > MIN_WIDTH ? tb_width() : MIN_WIDTH;

    widths->title_width = (int)(0.6 * width);
    widths->author_width = (int)(0.2 * width);
    widths->pub_date_width = (int)(0.2 * width);
}