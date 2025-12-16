#include "main_page.h"
#include "../ui_utils.h"

int main_menu(void) {
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