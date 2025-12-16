
// Prevent redefinitions by putting this before
// the definition of TB_IMPL
#include "ui.h"
#include "ui_utils.h" 
#include "pages/main_page.h"
#include "pages/feeds_page.h"

#define TB_IMPL

// Now after TB_IMPL is defined, the definitions will be included here.
#include "../termbox2/termbox2.h"
#include "../arena.h"

#include <stdarg.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>
#include <string.h>

// ------ Main UI Call ------ //
void ui_start(Channel **channel_list, const size_t channel_count) {
    tb_init();
    int y = 5;

    // Calculate how wide the feed columns need to be
    set_feed_column_widths();
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