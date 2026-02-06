#ifndef XML_RSS_H
#define XML_RSS_H

#include "node.h"
#include "../list.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

enum tag_type {
    TAG_OPEN,
    TAG_CLOSE,
    TAG_SELF_CLOSE,
};

typedef struct {
    char            *name;
    enum tag_type   tag_type;
    size_t          total_length;
} xml_tag;

typedef struct {
    int64_t         id;
    char            *title;
    char            *description;
    char            *link;
    char            *rss_link;
    int64_t         last_updated;
    char            *language;
    generic_list     *items; // List of items
} rss_channel;

#define MAX_RSS_ITEM_DATE_LEN 64

typedef struct {
    int64_t         id;
    char            *title;
    char            *author;
    int64_t         unix_timestamp;
    char            *description;
    char            *link;
    char            *guid;
    int             channel_id;
} rss_item;

enum container_type {
    CHANNEL,
    ITEM,
    // IMG, // Not going to worry about this for now. TODO: Add image support?
};

typedef struct {
    enum container_type     type;
    union {
        rss_item     *item;
        rss_channel  *channel;
    };
} rss_container;

rss_channel *build_channel(char *xml_rss, size_t size, char *link);
rss_channel *channel_init(void);
rss_item *item_init(void);

void free_channel(rss_channel *c);
void free_item(rss_item *it);
#endif