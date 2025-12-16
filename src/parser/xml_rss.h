#ifndef XML_RSS_H
#define XML_RSS_H

#include "node.h"
#include "../list.h"

#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>

enum TAG_TYPE {
    TAG_OPEN,
    TAG_CLOSE,
    TAG_SELF_CLOSE,
};

typedef struct Tag {
    char            *name;
    enum TAG_TYPE   tag_type;
    size_t          total_length;
} Tag;

typedef struct Item {
    char    *title;
    char    *author;
    char    *pub_date;
    char    *description;
    char    *link;
    char    *guid;
} Item;

typedef struct Channel {
    char            *title;
    char            *description;
    char            *link;
    char            *last_build_date;
    char            *language;
    List     *items; // List of items
} Channel;

enum CONTAINER_TYPE {
    CHANNEL,
    ITEM,
    // IMG, // Not going to worry about this for now. TODO: Add image support
};

typedef struct Container {
    enum CONTAINER_TYPE     type;
    union {
        Item     *item;
        Channel  *channel;
    };
} Container;

Node *construct_parse_tree(const char *xml, size_t length);

void print_parse_tree(const Node *root, int depth);

bool build_channel(Channel *chan, Node *parse_tree);

Channel *channel_init(void);
Item *item_init(void);

void free_channel(Channel *c);
void free_item(Item *it);
#endif