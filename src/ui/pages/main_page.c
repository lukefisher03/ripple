#include "handlers.h"
#include "../ui_utils.h"

static const char *main_menu_options[] = {
    "Channels",
    "Preferences",
    "Feedback",
    "Exit",
};

void main_menu(app_state *app, local_state *state) {
    (void) app;
    (void) state;

    int y = 5;
    y += display_logo((tb_width() / 2) - 14, y, TB_WHITE, TB_BLACK);
    y += 2;
    
    write_centered(y + 8, TB_GREEN, 0, "Made by Luke Fisher");

    int ret = display_basic_menu(y, main_menu_options, sizeof(char *), 4);

    // Switch to transition to the other pages
    switch (ret)
    {
    case 0:
        navigate(CHANNELS_PAGE, app, (local_state){});
        break;
    default:
        navigate(EXIT_PAGE, app, (local_state){});
        break;
    }
}

