#ifndef CHANNEL_DB_API
#define CHANNEL_DB_API

#include "../parser/xml_rss.h"
#include "../list.h"
#include "channels_database.h"

typedef enum HOST_OS {
    DARWIN,
    LINUX,
    UNSUPPORTED,
} HOST_OS;

typedef struct {
    rss_item    *item;
    char        *channel_name;
} article_with_channel_name;

int create_database_thread(void);
int db_tp_enqueue(rss_channel *channel);

int build_ripple_database(void);

void get_db_path(void);
int get_stale_channels(time_t cutoff, generic_list *out_list);
int get_channel_id(sqlite3 *db, const rss_channel *channel); 
int get_main_feed_articles(generic_list *article_list);
int get_article(int article_id, rss_item *article);
int get_channel(int channel_id, rss_channel *channel);
int get_channel_article_count(const rss_channel *channel, int *out_count);
int get_channel_articles(rss_channel *channel, generic_list *out_list);

int toggle_channel_visibility(int channel_id);

int delete_channel(int channel_id);

void free_article_with_channel_name(article_with_channel_name *article);
#endif