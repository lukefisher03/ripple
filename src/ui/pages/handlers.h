#ifndef UI_HANDLERS_H
#define UI_HANDLERS_H

#include "../ui.h"

void main_menu(app_state *app, local_state *state);

typedef struct channel_column_widths {
    int  channel_name;
    int  title;
    int  author;
    int  pub_date;
} channel_column_widths;

void channel_reader(app_state *app, local_state *state);
void channel_reader_destroy(void);

void article_page(app_state *app, local_state *state);
#endif