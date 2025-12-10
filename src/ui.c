#define TB_IMPL
#include "ui.h"
#include <stdarg.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>

#include "arena.h"

// ------ Forward declarations ------ //
static int main_menu(void);
static void feed_reader(void);

static int write_centered(int y, uintattr_t fg, uintattr_t bg, char *text, ...);
static int display_menu(int y, char **options, int option_count, void (*render_selection)(struct selection *));
static int display_logo(int x, int y, uintattr_t fg, uintattr_t bg);
static void render_main_menu_selection(struct selection *s);
// ------ UI ------ //
void ui_start() {
    struct tb_event ev;
    int y = 5;

    tb_init();
    
    // Show initial menu
    int selection = main_menu();
    
    // Select Menu Option
    switch (selection)
    {
    case OPTION_FEEDS:
        printf("Selected FEEDS!\n");
        feed_reader();
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
    char *options[] = {"Read Feeds", "Preferences", "Feedback", "Exit"};

    write_centered(y + 8, TB_GREEN, 0, "Made by Luke Fisher");

    int ret = display_menu(y, options, 4, &render_main_menu_selection);
    tb_present();
    return ret;
}

static void feed_reader(void) {
    struct arena arena;
    struct tb_event ev;
    int width = tb_width();
    arena_init(&arena, 4096);

    tb_clear();
    int y = 0;
    char *divider = arena_allocate(&arena, tb_width());
    for (size_t i = 0; i < width; i++) {
        divider[i] = '-';
    }
    tb_printf(0, y++, TB_GREEN, 0, "Feed Reader");
    tb_printf(0, y++, TB_GREEN, 0, divider);
    tb_present();
    tb_poll_event(&ev);
    arena_free(&arena);
}

static int display_menu(int y, 
                        char **options, 
                        int option_count, 
                        void (*render_selection)(struct selection *)
                       ) 
{
    // Display a menu and then return the selected option
    int selected_option = 0;
    int new_y = y;
    while (1) {
        struct tb_event ev;
        for (int i = 0; i < option_count; i++) {
            struct selection selection = {
                .x = 0,
                .y = new_y++,
                .selected = selected_option == i,
                .text = options[i],
            };

            render_selection(&selection);
        }
        tb_present();
        tb_poll_event(&ev);
        switch (ev.key) {
            case TB_KEY_ENTER:
                return selected_option;
            case TB_KEY_ARROW_UP:
                selected_option = selected_option > 0 ? selected_option - 1 : 0; 
                break;
            case TB_KEY_ARROW_DOWN:
                selected_option = selected_option < option_count - 1 ? selected_option + 1 : selected_option;
                break;
            default:
                break;
        }
        new_y = y;
    }
}

static int write_centered(int y, uintattr_t fg, uintattr_t bg, char *text, ...) {
    size_t text_len = strlen(text);
    int mid = tb_width() / 2;
    int v = tb_printf((mid - text_len / 2), y, fg, bg, text);
    return v;
}

static void render_main_menu_selection(struct selection *s) {
    // There's potentially a better way to do this, but this works for now
    char *selected_s = malloc(strlen(s->text) + 4); 

    if (!selected_s) {
        fprintf(stderr, "Memory allocation failed for rendering selection.\n");
        return;
    } 

    if (s->selected) {
        sprintf(selected_s, "[ %s ]", s->text);
    } else {
        sprintf(selected_s, "  %s  ", s->text);
    }
    write_centered(s->y, TB_GREEN, 0, selected_s);
    free(selected_s);
}

static void render_feed_article_selections(struct selection *s) {
    int width = tb_width();
    if (s->selected) {

    } else {

    }
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