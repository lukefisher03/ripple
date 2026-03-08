#include "handlers.h"
#include "../../logger.h"
#include "../ui_utils.h"
#include "../../channel_manager.h"

void refresh_page(app_state *app, local_state *state) {
    char *options[2];
    options[0] = "yes";
    options[1] = "no";

    char msg[256];
    int n = snprintf(msg, sizeof(msg), "Channels older than %d hour/s will be refreshed, continue?", REFRESH_PERIOD_HOURS);

    if (n < 0 || n >= sizeof(msg)) {
        log_debug("Error copying message, buffer too small on refresh page. Message was truncated");
    }
    
    menu_result result = display_confirmation_menu(msg, options, 2);
    if (result.selection == 0) {
        tb_clear();
        write_centered(tb_height() / 2 - 1, TB_GREEN, 0, "Refreshing channels...");
        tb_present();
        refresh_channels();
    }
    navigate(MAIN_PAGE, app, (local_state){});
}