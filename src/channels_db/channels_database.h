#ifndef CHANNELS_DB_H 
#define CHANNELS_DB_H 

#include "../parser/xml_rss.h"
#include <sqlite3.h>
#include <stdlib.h>

int create_channel_table(sqlite3 *db, char **err_msg);
int read_channel_from_stmt(sqlite3_stmt *stmt, rss_channel *channel);
int read_article_from_stmt(sqlite3_stmt *stmt, rss_item *item);
int create_article_table(sqlite3 *db, char **err_msg);

int store_channel(sqlite3 *db, const rss_channel *channel);
int store_article(sqlite3 *db, const rss_item *item, const rss_channel *channel);

int get_channel_list(generic_list *article_list);

#endif