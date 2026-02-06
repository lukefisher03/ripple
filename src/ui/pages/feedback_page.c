#include "handlers.h"
#include "../ui_utils.h"
#include "../ui.h"

void feedback_page(app_state *app, local_state *state) {
    int y = 5;
    write_centered(y++, TB_GREEN, 0, "Feedback");
    y+= 2;
    write_centered(y++, TB_GREEN, 0, "Feel free to leave issues on the github repo.");
    write_centered(y++, TB_GREEN, 0, "https://github.com/lukefisher03/ripple/issues");

    y += 5;
    char *options[2];
    options[0] = "back";
    options[1] = "exit";

    menu_result result = display_basic_menu(y, options, 2);
    if (result.selection == 1) {
        navigate(EXIT_PAGE, app, (local_state){});
        return;
    }
    navigate(MAIN_PAGE, app, (local_state){});
}