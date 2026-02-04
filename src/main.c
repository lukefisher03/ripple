#include "ui/ui.h"
#include "logger.h"
#include "utils.h"
#include "channels_db/channels_database.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    log_init();
    if (build_ripple_database() != 0) {
        fprintf(stderr, "Failed to create db, exiting.\n");
        return 1;
    }
    ui_start();
    log_close();
}
