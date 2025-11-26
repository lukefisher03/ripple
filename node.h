#ifndef NODE_H
#define NODE_H

#include "dynamic_string.h"
#include "list.h"

struct xml_node {
    struct list         *children;      // Number of children.
    char                *name;          // Name of the element.
};

enum NODE_TYPE {
    XML_NODE,
    TEXT_NODE,
};

struct node {
    enum NODE_TYPE type;
    union {
        struct xml_node     xml;
        char                *text; // Text nodes just hold text
    };
};

struct node *xml_node_init();
struct node *text_node_init();

#endif