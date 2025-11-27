#ifndef NODE_H
#define NODE_H

#include "dynamic_string.h"
#include "list.h"

enum NODE_TYPE {
    ROOT_NODE,
    XML_NODE,
    TEXT_NODE,
    DUMMY,
};

struct xml_node {
    struct list         *children;      // Children are strictly other nodes
    char                *name;          // Name of the element.
};

struct node {
    enum NODE_TYPE type;
    union {
        struct xml_node     xml;
        char                *text; // Text nodes just hold text
    };
};

struct node *xml_node_init(void);
struct node *text_node_init(void);
struct node *dummy_node_init(void);

void free_node(struct node *node);
void free_tree(struct node *node);

#endif