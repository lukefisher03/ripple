#include "handlers.h"
#include "../ui_utils.h"
#include "../../logger.h"

static const char *main_menu_options[] = {
    "feed",
    "channels",
    "preferences",
    "feedback",
    "exit",
};

void main_menu(app_state *app, local_state *state) {
    (void) app;
    (void) state;

    int y = 5;
    y += print_logo((tb_width() / 2) - 14, y, TB_WHITE, TB_BLACK);
    y += 2;
    
    write_centered(y + 8, TB_GREEN, 0, "Made by Luke Fisher");

    int option_count = sizeof(main_menu_options) / sizeof(main_menu_options[0]);
    menu_result result = display_basic_menu(y, main_menu_options, option_count);
    
    // Switch to transition to the other pages
    switch (result.selection)
    {
    case 0:
        navigate(FEED_PAGE, app, (local_state){});
        break;
    case 1:
        navigate(CHANNELS_PAGE, app, (local_state){});
        break;
    default:
        navigate(EXIT_PAGE, app, (local_state){});
        break;
    }
}

