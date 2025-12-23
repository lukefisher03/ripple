#include "ui_utils.h"

static int render_basic_menu(int x, int y, bool selected, const void *txt);

int display_menu(int y, 
                 const void *options, 
                 size_t option_size,
                 int option_count, 
                 option_renderer render_selection 
) 
{
    // Display a menu and then return the selected option
    int cursor = 0;
    int screen_height = tb_height();
    int new_y = y;

    int option_height = 1;
    int window = (screen_height - y) / option_height;
    int offset = 0;

    while (1) {
        struct tb_event ev;
        for (int i = offset; i < option_count && i < offset + window; i++) {
            // Convert void pointer to char * so we can do pointer arithmetic.
            // add the index multiplied by the size to increment the pointer.
            void *option = (char *)options + (i * option_size);

            option_height = render_selection(0, new_y, i == cursor, option);

            new_y += option_height;
            window = ((screen_height - y) / option_height);
        }

        tb_present();
        tb_poll_event(&ev);

        if (ev.key == TB_KEY_ENTER) return cursor;
        if (ev.key == TB_KEY_ARROW_UP || ev.ch == 'k') {
            if (cursor > 0) {
                cursor--;
                if (cursor < offset) {
                    offset--;
                }
            }
        }
        if (ev.key == TB_KEY_ARROW_DOWN || ev.ch == 'j') {
            if (cursor < option_count - 1) {
                cursor++;
                if (cursor >= offset + window ) {
                    offset++;
                }
            }
        }

        new_y = y;
    }
}

int display_basic_menu(
                int y, 
                const void *options, 
                size_t option_size,
                int option_count 
) {
    return display_menu(y, options, option_size, option_count, &render_basic_menu);
}

int write_centered(int y, uintattr_t fg, uintattr_t bg, const char *text) {
    size_t text_len = strlen(text);
    int mid = tb_width() / 2;
    int v = tb_printf((mid - text_len / 2), y, fg, bg, text);
    return v;
}

static int render_basic_menu(int x, int y, bool selected, const void *txt) {
    (void) x; // x is not used for this selection renderer
    char *text = *(char **)txt;
    // There's potentially a better way to do this, but this works for now
    int new_y = y;
    char *selected_s = malloc(strlen(text) + 10); 

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


int display_logo(int x, int y, uintattr_t fg, uintattr_t bg) {
    int new_y = y;

    tb_printf(x, new_y++, fg, bg, "    ___  _           __    "); 
    tb_printf(x, new_y++, fg, bg, "   / _ \\(_)__  ___  / /__  ");
    tb_printf(x, new_y++, fg, bg, "  / , _/ / _ \\/ _ \\/ / -_) ");
    tb_printf(x, new_y++, fg, bg, " /_/|_/_/ .__/ .__/_/\\__/  ");
    tb_printf(x, new_y++, fg, bg, "       /_/  /_/            ");

    return new_y - y;
}
