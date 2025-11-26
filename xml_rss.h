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
};

struct channel {
    char            *title;
    char            *description;
    char            *link;
    char            *last_build_date;
    struct item     **items;
};


struct node *construct_parse_tree(char *xml, size_t length);

void print_parse_tree(struct node *root, int depth);

bool read_tag(char *str, size_t length, struct tag *t);

ssize_t accumulate_text(char *str, size_t length, struct node *new_node);

bool is_termination_char(char c);

struct tag *tag_init();

bool build_channel(struct channel *chan, struct node *parse_tree);

char * get_spacer(int width);
#endif