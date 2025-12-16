#include "utils.h"
#include "list.h"
#include "parser/xml_rss.h"
#include "parser/node.h"
#include "ui/ui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    size_t size;
    // char *rss = file_to_string("test/smart_less.xml", &size);
    char *rss = file_to_string("test/stack_overflow.xml", &size);
    Node *tree = construct_parse_tree(rss, size);
    free(rss);
    Channel *c = channel_init();
    build_channel(c, tree);
    free_tree(tree);

    Channel **channel_list = calloc(1, sizeof(*channel_list));
    channel_list[0] = c;
    ui_start(channel_list, 1);
    free_channel(c);
}
