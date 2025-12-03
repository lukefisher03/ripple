#define TB_IMPL
#include "ui.h"
#include <stdarg.h>

// ------ Forward declarations ------ //
int write_centered(int y, uintattr_t fg, uintattr_t bg, char *text, ...);
int display_menu(int y, char **options, int option_count);

// ------ UI ------ //
void ui_start() {
    struct tb_event ev;
    int y = 5;

    tb_init();
    
    write_centered(y++, TB_GREEN, 0, "RIPPLE");
    tb_present();
    y += 5;
    char *options[] = {"Read Feeds", "Preferences", "Feedback"};
    int ret = display_menu(y, options, 3);
    char s[100];
    sprintf(s, "Selected option: %i\n", ret);
    write_centered(20, TB_GREEN, 0, s);
    tb_present();
    tb_poll_event(&ev);
    tb_shutdown();
}

int display_menu(int y, char **options, int option_count) {
    int selected_option = 0;
    int new_y = y;
    while (1) {
        struct tb_event ev;
        for (int i = 0; i < option_count; i++) {
            if (i == selected_option) {
                write_centered(new_y++, TB_BLACK, TB_WHITE, options[i]);
            } else {
                write_centered(new_y++, TB_GREEN, 0, options[i]);
            }
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

int write_centered(int y, uintattr_t fg, uintattr_t bg, char *text, ...) {
    size_t text_len = strlen(text);
    int mid = tb_width() / 2;
    int v = tb_printf((mid - text_len / 2), y, fg, bg, text);
    return v;
}