#include "utils.h"
#include "list.h"
#include "parser/xml_rss.h"
#include "parser/node.h"
#include "ui/ui.h"
#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    // Initialize the logger
    log_init();
    ui_start();
}
