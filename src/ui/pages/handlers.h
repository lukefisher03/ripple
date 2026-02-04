#ifndef UI_HANDLERS_H
#define UI_HANDLERS_H

#include "../ui.h"

// ----- MAIN MENU PAGE ------ //
void main_menu(app_state *app, local_state *state);
void main_feed_destroy(void);
// ----- FEED PAGE ------ //
void main_feed(app_state *app, local_state *state);
typedef struct feed_column_widths {
    int  channel_name;
    int  title;
    int  author;
    int  pub_date;
} feed_column_widths;

// ----- ARTICLE PAGE ------ //
void article_page(app_state *app, local_state *state);

// ----- CHANNELS PAGE ------ //
void manage_channels_page(app_state *app_state, local_state *state);
typedef struct channel_column_widths {
    int channel_name;
    int article_count;
    int last_updated;
} channel_column_widths;

// ----- INDIVIDUAL CHANNEL PAGE ------ //
void channel_page(app_state *app_state, local_state *state);
typedef struct channel_page_column_widths {
    int title;
    int author;
    int date;
} channel_page_column_widths;

// ----- CONFIRMATION PAGE ------ //
#define CONFIRMATION_MSG_SIZE 2048
typedef struct channel_with_extras {
    rss_channel     *chan;
    int             article_count;
} channel_with_extras;

#endif