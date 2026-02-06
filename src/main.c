#include "ui/ui.h"
#include "logger.h"
#include "utils.h"
#include "channels_db/channels_database.h"
#include "channel_manager.h"
#include "list.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int get_new_channel_links(const char *feeds_file, size_t length, generic_list *list);

int main(int argc, char *argv[]) {
    log_init();
    get_db_path();

    for(size_t i = 0; i < argc; i++) {
        log_debug("ARG %i: %s", i + 1, argv[i]);
    }

    if (build_ripple_database() != 0) {
        fprintf(stderr, "Failed to create db, exiting.\n");
        return 1;
    }

    initial_state init_state = {0};

    init_state.new_channel_links_file_path = argc > 1 ? argv[1] : NULL;

    ui_start(init_state);
    log_close();
}



