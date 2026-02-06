#ifndef UI_HANDLERS_H
#define UI_HANDLERS_H

#include "../ui.h"

// ----- MAIN MENU PAGE ------ //
void main_menu(app_state *app, local_state *state);

// ----- FEED PAGE ------ //
void main_feed(app_state *app, local_state *state);
typedef struct {
    float channel_name;
    float title;
    float pub_date;
} feed_column_widths;

// ----- ARTICLE PAGE ------ //
void article_page(app_state *app, local_state *state);

// ----- CHANNELS PAGE ------ //
void manage_channels_page(app_state *app, local_state *state);
typedef struct {
    float channel_name;
    float article_count;
    float last_updated;
} channel_column_widths;

// ----- INDIVIDUAL CHANNEL PAGE ------ //
void channel_page(app_state *app, local_state *state);
typedef struct {
    float title;
    float author;
    float date;
} channel_page_column_widths;

// ----- CONFIRMATION PAGE ------ //
#define CONFIRMATION_MSG_SIZE 2048
typedef struct {
    rss_channel     *chan;
    int             article_count;
} channel_with_extras;

// ----- IMPORT PAGE ------ //
void import_page(app_state *app, local_state *state);

// ----- REFRESH PAGE ------ //
void refresh_page(app_state *app, local_state *state);

// ----- FEEDBACK PAGE ------ //
void feedback_page(app_state *app, local_state *state);

#endif