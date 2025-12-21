#ifndef NODE_H
#define NODE_H

#include "../list.h"

typedef enum node_type {
    ROOT_NODE,
    XML_NODE,
    TEXT_NODE,
    DUMMY,
} node_type;

typedef struct xml_rss_node {
    struct generic_list         *children;      // Children are strictly other nodes
    char                *name;          // Name of the element.
} xml_rss_node;

typedef struct rss_node {
    enum node_type type;
    union {
        struct xml_rss_node     xml;
        char                *text; // Text nodes just hold text
    };
} rss_node;

rss_node *xml_node_init(void);
rss_node *text_node_init(void);
rss_node *dummy_node_init(void);

void free_node(rss_node *node);
void free_tree(rss_node *node);

#endif