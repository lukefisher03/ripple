#include "ui/ui.h"
#include "logger.h"

int main(int argc, char *argv[]) {
    // Initialize the logger
    log_init();
    ui_start();
    log_close();
}
