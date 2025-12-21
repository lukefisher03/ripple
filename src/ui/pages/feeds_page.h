#ifndef FEEDS_PAGE_H
#define FEEDS_PAGE_H

#include "../../termbox2/termbox2.h"
#include "../../parser/xml_rss.h"

// Forward declare app_state struct
typedef struct app_state app_state;

typedef struct channel_column_widths {
    size_t  channel_name;
    size_t  title;
    size_t  author;
    size_t  pub_date;
} channel_column_widths;

typedef struct feeds_page_state {
    rss_channel **channel_list;
    size_t      channel_count;
} feeds_page_state;

void feed_reader(app_state *app);
void set_feed_column_widths(void);

#endif