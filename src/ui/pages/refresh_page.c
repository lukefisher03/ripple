#include "handlers.h"
#include "../ui_utils.h"
#include "../../channel_manager.h"

void refresh_page(app_state *app, local_state *state) {
    char *options[2];
    options[0] = "yes";
    options[1] = "no";
    menu_result result = display_confirmation_menu("Refresh channels older than an hour?", options, 2);
    if (result.selection == 0) {
        tb_clear();
        write_centered(tb_height() / 2 - 1, TB_GREEN, 0, "Refreshing channels...");
        tb_present();
        refresh_channels();
    }
    navigate(MAIN_PAGE, app, (local_state){});
}