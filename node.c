#include "node.h"
#include <stdio.h>

struct node *xml_node_init() {
    struct node *new_node = calloc(1, sizeof(*new_node));
    if (!new_node) {
        return NULL;
    }
    new_node->type = XML_NODE;
    new_node->xml.children = list_init();
    return new_node;
}

struct node *text_node_init() {
    struct node *new_node = calloc(1, sizeof(*new_node));
    if (!new_node) {
        return NULL;
    }

    new_node->type = TEXT_NODE;
    new_node->text = NULL;
    return new_node;
}
