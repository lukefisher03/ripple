#ifndef CHANNEL_DB_API
#define CHANNEL_DB_API

#include "../parser/xml_rss.h"
#include "../list.h"
#include "channels_database.h"

#define DB_PATH "ripple.db"

typedef struct article_with_channel_name {
    rss_item    *item;
    char        *channel_name;
} article_with_channel_name;

void free_article_with_channel_name(article_with_channel_name *article);
int build_ripple_database(void);
int store_channel_list(size_t channel_count, rss_channel **channels);
int get_main_feed_articles(generic_list *article_list);
int get_article(int article_id, rss_item *article);
int get_channel_count(sqlite3 *db);
int get_channel(int channel_id, rss_channel *channel);
int get_channel_article_count(const rss_channel *channel, int *out_count);
int get_channel_articles(rss_channel *channel, generic_list *out_list);
int delete_channel(int channel_id);

#endif