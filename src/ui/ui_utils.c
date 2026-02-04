#include "ui_utils.h"
#include "../logger.h"
#include "../termbox2/termbox2.h"
#include "ui.h"

#include <assert.h>

int COL_GAP = 3;
char *thin_divider = NULL;
char *blank_line = NULL;
char *thick_divider = NULL;

static int render_basic_menu(int x, int y, bool selected, const void *txt);

void reset_dividers() {
    free(thin_divider);
    free(thick_divider);
    free(blank_line);

    int width = tb_width();
    
    thin_divider = calloc(width + 1, sizeof(char));
    blank_line = calloc(width + 1, sizeof(char));
    thick_divider = calloc(width + 1, sizeof(char));
    
    if (!thin_divider || !blank_line || !thick_divider) {
        log_debug("Failed to allocate space for dividers");
        abort();
    }

    memset(thin_divider, '-', width);
    memset(blank_line, ' ', width);
    memset(thick_divider, '=', width);

    thin_divider[width] = '\0';
    blank_line[width] = '\0';
    thick_divider[width] = '\0';
}

menu_result display_menu(int y, 
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

            bool selected = i == cursor;
            option_height = render_selection(0, new_y, selected, option);

            new_y += option_height;
            window = ((screen_height - y) / option_height);
        }

        tb_present();
        tb_poll_event(&ev);

        if (ev.key == TB_KEY_ENTER) return (menu_result) {
            .selection = cursor,
            .ev = ev,
        };
        if (ev.key == TB_KEY_ARROW_UP || ev.ch == 'k') {
            if (cursor > 0) {
                cursor--;
                if (cursor < offset) {
                    offset--;
                }
            }
        } else if (ev.key == TB_KEY_ARROW_DOWN || ev.ch == 'j') {
            if (cursor < option_count - 1) {
                cursor++;
                if (cursor >= offset + window ) {
                    offset++;
                }
            }
        } else {
            return (menu_result) {
                .selection = cursor,
                .ev = ev,
            };
        }

        new_y = y;
    }
}

menu_result display_basic_menu(
                int y, 
                const void *options, 
                int option_count
) {
    return display_menu(y, options, sizeof(char *), option_count, &render_basic_menu);
}

menu_result display_confirmation_menu(const char *msg, char **options, int option_count) {
    tb_clear();
    int y = 4;
    write_centered(y++, TB_GREEN, 0, msg);

    y += 5;
    return display_basic_menu(y, options, option_count);
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
    size_t max_len = strlen(text) + 5;
    char *selected_s = malloc(max_len); 

    if (!selected_s) {
        log_debug("Failed memory allocation");
        return 0;
    } 

    if (selected) {
        snprintf(selected_s, max_len,"[ %s ]", text);
        for (size_t i = 0; i < max_len; i++) {
            char ch = selected_s[i];
            selected_s[i] = toupper((unsigned char)ch);
        }
    } else {
        snprintf(selected_s, max_len, "  %s  ", text);
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

size_t add_column(char *row, int col_width, const char *col_str) {
    assert(row != NULL);
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
