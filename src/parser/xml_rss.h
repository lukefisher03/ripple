#ifndef XML_RSS_H
#define XML_RSS_H

#include "node.h"
#include "../list.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>

enum tag_type {
    TAG_OPEN,
    TAG_CLOSE,
    TAG_SELF_CLOSE,
};

typedef struct Tag {
    char            *name;
    enum tag_type   tag_type;
    size_t          total_length;
} Tag;

#define MAX_RSS_ITEM_DATE_LEN 64

typedef struct rss_item {
    char        *title;
    char        *author;
    // We store the pub date as a string as a backup
    char        *pub_date_rfc822;
    // Max date string length
    char        pub_date_string[MAX_RSS_ITEM_DATE_LEN];
    long        pub_date_unix;
    char        *description;
    char        *link;
    char        *guid;
} rss_item;

typedef struct rss_channel {
    char            *title;
    char            *description;
    char            *link;
    char            *last_build_date;
    char            *language;
    generic_list     *items; // List of items
} rss_channel;

enum container_type {
    CHANNEL,
    ITEM,
    // IMG, // Not going to worry about this for now. TODO: Add image support
};

typedef struct rss_container {
    enum container_type     type;
    union {
        rss_item     *item;
        rss_channel  *channel;
    };
} rss_container;

rss_node *construct_parse_tree(const char *xml, size_t length);

void print_parse_tree(const rss_node *root, int depth);

bool build_channel(rss_channel *chan, rss_node *parse_tree);
rss_channel **load_channels(char *files[], size_t size);
rss_channel *channel_init(void);
rss_item *item_init(void);

void free_channel(rss_channel *c);
void free_item(rss_item *it);
#endif