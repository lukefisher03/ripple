#ifndef FEEDS_PAGE_H
#define FEEDS_PAGE_H

#include "../../termbox2/termbox2.h"
#include "../../parser/xml_rss.h"
#include "../../arena.h"

typedef struct channel_column_widths {
    size_t  title_width;
    size_t  author_width;
    size_t  pub_date_width;
} channel_column_widths;


void feed_reader(Channel **channel_list);
void set_feed_column_widths(void);

#endif