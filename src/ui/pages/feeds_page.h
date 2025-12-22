#ifndef FEEDS_PAGE_H
#define FEEDS_PAGE_H

#include "../../termbox2/termbox2.h"
#include "../../parser/xml_rss.h"
#include "../ui.h"

typedef struct channel_column_widths {
    size_t  channel_name;
    size_t  title;
    size_t  author;
    size_t  pub_date;
} channel_column_widths;

void feed_reader(app_state *app);
void feed_reader_destroy(void);
void set_feed_column_widths(void);

#endif