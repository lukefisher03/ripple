#include "handlers.h"
#include "../ui_utils.h"
#include "../ui.h"

void confirmation_page(app_state *app, local_state *state) {
    int y = 4;
    write_centered(y++, TB_GREEN, 0, state->confirmation_state.message);
    y += 3;
    display_basic_menu(y, state->confirmation_state.options, state->confirmation_state.option_count);
    // navigate(app->previous_page.type, app, (local_state){});
    navigate(MAIN_PAGE, app, (local_state){});
}