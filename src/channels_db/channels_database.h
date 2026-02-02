#ifndef CHANNELS_DB_H 
#define CHANNELS_DB_H 

#include "../parser/xml_rss.h"
#include <sqlite3.h>

int create_channel_table(sqlite3 *db, char **err_msg);
int create_article_table(sqlite3 *db, char **err_msg);
int create_metadata_table(sqlite3 *db, char **err_msg);

int store_channel(sqlite3 *db, const rss_channel *channel);
int store_article(sqlite3 *db, const rss_item *item, const rss_channel *channel);

int get_channel_list(generic_list *article_list);
#endif