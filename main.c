#include "utils.h"
#include "xml_rss.h"
#include "node.h"
#include "dynamic_string.h"
#include "list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    size_t size;
    char *rss = file_to_string("test/smart_less.xml", &size);
    // char *rss = file_to_string("test/stack_overflow.xml", &size);
    struct node *tree = construct_parse_tree(rss, size);
    free(rss);
    // print_parse_tree(tree, 0);
    struct channel *c = channel_init();
    build_channel(c, tree);
    free_tree(tree);
    printf("Channel description: %s\n", c->description);
    for (size_t i = 0; i < c->items->count; i++) {
        struct item *it = c->items->elements[i];
        printf("title: %s\n", it->title);
        printf("guid: %s\n", it->guid);
    }

    free_channel(c);
}