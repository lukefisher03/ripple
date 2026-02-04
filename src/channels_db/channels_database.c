#include "channels_database.h"
#include "../logger.h"

#include <sqlite3.h>
#include <stdio.h>
#include <string.h>

int create_channel_table(sqlite3 *db, char **err_msg) {
    if (!db) return 1;
    *err_msg = NULL;

    char *create_feeds_table_cmd =  "CREATE TABLE IF NOT EXISTS channel("
                                    "id INTEGER PRIMARY KEY," // NOTE: You must use INTEGER not INT for the primary key. 
                                    "title TEXT NOT NULL,"
                                    "description TEXT,"
                                    "language TEXT,"
                                    "link TEXT UNIQUE NOT NULL,"
                                    "last_updated INTEGER DEFAULT (unixepoch()));";

    int result = sqlite3_exec(db, create_feeds_table_cmd, NULL, NULL, err_msg);
    if (result != SQLITE_OK) {
        fprintf(stderr, "Error: Failed to create channels table - %s\n", *err_msg);
        sqlite3_free(*err_msg);
        *err_msg = NULL;
    }

    return result;
} 

int create_article_table(sqlite3 *db, char **err_msg) {
    if (!db) return 1;
    *err_msg = NULL;
    
    char *create_articles_table_cmd =   "CREATE TABLE IF NOT EXISTS article(" 
                                        "id INTEGER PRIMARY KEY," 
                                        "title TEXT UNIQUE NOT NULL," 
                                        "author TEXT," 
                                        "description TEXT,"
                                        "link TEXT NOT NULL," 
                                        "unix_timestamp INTEGER NOT NULL,"
                                        "channel_id INTEGER,"
                                        "UNIQUE(channel_id, title, unix_timestamp),"
                                        "FOREIGN KEY(channel_id) REFERENCES channel(id) ON DELETE CASCADE);";
    int result = sqlite3_exec(db, create_articles_table_cmd, NULL, NULL, err_msg);

    if (result != SQLITE_OK) {
        fprintf(stderr, "Error: Failed to build articles table - %s\n", *err_msg);
        sqlite3_free(*err_msg);
        *err_msg = NULL;
    }

    return result;
}

int store_article(sqlite3 *db, const rss_item *item, const rss_channel *channel) {
    if (!item || !db) {
        return 1;
    }

    const char *insert_article_sql = "INSERT INTO article(title, author, description, link, unix_timestamp, channel_id) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt = NULL;
    int result = 1;

    result = sqlite3_prepare_v2(db, insert_article_sql, -1, &stmt, NULL);
    if (result != SQLITE_OK) goto cleanup;

    int argument_idx = 1;

    result = sqlite3_bind_text(stmt, argument_idx++, item->title, -1, SQLITE_TRANSIENT);
    if (result != SQLITE_OK) goto cleanup;
    result = sqlite3_bind_text(stmt, argument_idx++, item->author, -1, SQLITE_TRANSIENT);
    if (result != SQLITE_OK) goto cleanup;
    result = sqlite3_bind_text(stmt, argument_idx++, item->description, -1, SQLITE_TRANSIENT);
    if (result != SQLITE_OK) goto cleanup;
    result = sqlite3_bind_text(stmt, argument_idx++, item->link, -1, SQLITE_TRANSIENT);
    if (result != SQLITE_OK) goto cleanup;
    result = sqlite3_bind_int(stmt, argument_idx++, item->unix_timestamp);
    if (result != SQLITE_OK) goto cleanup;
    result = sqlite3_bind_int(stmt, argument_idx++, channel->id);
    if (result != SQLITE_OK) goto cleanup;

    result = sqlite3_step(stmt);

    cleanup:
        if (result != SQLITE_DONE && result != SQLITE_OK) {
            log_debug("DB Error storing article, %s", sqlite3_errmsg(db));
        } else {
            result = SQLITE_OK;
        }
        sqlite3_finalize(stmt);
    return result;
}

int store_channel(sqlite3 *db, const rss_channel *channel) {
    if (!channel || !db) {
        return 1;
    }

    char *insert_channel_sql = "INSERT INTO channel(title, description, language, link) VALUES (?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    int result;

    result = sqlite3_prepare_v2(db, insert_channel_sql, -1, &stmt, NULL);
    if (result != SQLITE_OK) goto cleanup;

    int argument_idx = 1;
     
    result = sqlite3_bind_text(stmt, argument_idx++, channel->title, -1, NULL);
    if (result != SQLITE_OK) goto cleanup;
    result = sqlite3_bind_text(stmt, argument_idx++, channel->description, -1, NULL);
    if (result != SQLITE_OK) goto cleanup;
    result = sqlite3_bind_text(stmt, argument_idx++, channel->language, -1, NULL);
    if (result != SQLITE_OK) goto cleanup;
    result = sqlite3_bind_text(stmt, argument_idx++, channel->link, -1, NULL);
    if (result != SQLITE_OK) goto cleanup;

    result = sqlite3_step(stmt);

    cleanup:
        if (result != SQLITE_DONE && result != SQLITE_OK) {
            log_debug("DB Error failed to store channel, %s, %d", sqlite3_errmsg(db), result);
        } else {
            result = SQLITE_OK;
        }
        sqlite3_finalize(stmt);

    return result;
}

int read_channel_from_stmt(sqlite3_stmt *stmt, rss_channel *channel) {
    int arg_idx = 0;

    channel->id = sqlite3_column_int(stmt, arg_idx++);

    const unsigned char *title = sqlite3_column_text(stmt, arg_idx++);
    channel->title = title ? strdup((const char *)title) : NULL;

    const unsigned char *description = sqlite3_column_text(stmt, arg_idx++);
    channel->description = description ? strdup((const char *)description) : NULL;

    const unsigned char *language = sqlite3_column_text(stmt, arg_idx++);
    channel->language = language ? strdup((const char *)language) : NULL;

    const unsigned char *link = sqlite3_column_text(stmt, arg_idx++);
    channel->link = link ? strdup((const char *)link) : NULL;

    channel->last_updated = sqlite3_column_int(stmt, arg_idx++);

    return arg_idx;
}

int read_article_from_stmt(sqlite3_stmt *stmt, rss_item *item) {
    int arg_idx = 0;

    item->id = sqlite3_column_int(stmt, arg_idx++);
    
    const unsigned char* title = sqlite3_column_text(stmt, arg_idx++);
    item->title = title ? strdup((const char *)title) : NULL;

    const unsigned char* author = sqlite3_column_text(stmt, arg_idx++);
    item->author = author ? strdup((const char *)author) : NULL;
    
    const unsigned char* description = sqlite3_column_text(stmt, arg_idx++);
    item->description = description ? strdup((const char *)description) : NULL;

    const unsigned char* link = sqlite3_column_text(stmt, arg_idx++);
    item->link = link ? strdup((const char *)link) : NULL;
    
    item->unix_timestamp = sqlite3_column_int(stmt, arg_idx++);
    item->channel_id = sqlite3_column_int(stmt, arg_idx++);
    return arg_idx;
}