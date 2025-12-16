#ifndef NODE_H
#define NODE_H

#include "../list.h"

typedef enum NODE_TYPE {
    ROOT_NODE,
    XML_NODE,
    TEXT_NODE,
    DUMMY,
} NODE_TYPE;

typedef struct XMLNode {
    struct List         *children;      // Children are strictly other nodes
    char                *name;          // Name of the element.
} XMLNode;

typedef struct Node {
    enum NODE_TYPE type;
    union {
        struct XMLNode     xml;
        char                *text; // Text nodes just hold text
    };
} Node;

Node *xml_node_init(void);
Node *text_node_init(void);
Node *dummy_node_init(void);

void free_node(Node *node);
void free_tree(Node *node);

#endif