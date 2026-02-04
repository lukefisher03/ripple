#ifndef UI_STATES_H
#include "../../parser/xml_rss.h"
#include "../../channels_db/channel_db_api.h"

typedef struct article_page_state {
    int article_id;
} article_page_state;

typedef struct confirmation_page_state {
    char *message;
    char **options;
    size_t option_count;
} confirmation_page_state;

typedef struct channel_page_state {
    int channel_id;
} channel_page_state;
#endif