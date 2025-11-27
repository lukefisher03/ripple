#ifndef XML_RSS_H
#define XML_RSS_H

#include "list.h"
#include "node.h"
#include "dynamic_string.h"

#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>

enum TAG_TYPE {
    TAG_OPEN,
    TAG_CLOSE,
    TAG_SELF_CLOSE,
};

struct tag {
    char            *name;
    enum TAG_TYPE   tag_type;
    size_t          total_length;
};

struct item {
    char    *title;
    char    *author;
    char    *pub_date;
    char    *description;
    char    *link;
    char    *guid;
};

struct channel {
    char            *title;
    char            *description;
    char            *link;
    char            *last_build_date;
    char            *language;
    struct list     *items; // List of items
};

enum CONTAINER_TYPE {
    CHANNEL,
    ITEM,
    // IMG, // Not going to worry about this for now. TODO: Add image support
};

struct container {
    enum CONTAINER_TYPE     type;
    union {
        struct item     *item;
        struct channel  *channel;
    };
};

struct node *construct_parse_tree(const char *xml, size_t length);

void print_parse_tree(const struct node *root, int depth);

bool build_channel(struct channel *chan, struct node *parse_tree);

struct channel *channel_init(void);
struct item *item_init(void);

void free_channel(struct channel *c);
void free_item(struct item *it);
#endif