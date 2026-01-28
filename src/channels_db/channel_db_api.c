#include "channel_db_api.h"
#include "../logger.h"

#include <sqlite3.h>
#include <stdio.h>
#include <string.h>

int db_open(sqlite3 **db) {
    char *err_msg = NULL;
    int result = 1;

    result = sqlite3_open(DB_PATH, db);
    if (result != SQLITE_OK) {
        fprintf(stderr, "Error connecting to database: %s\n", sqlite3_errmsg(*db));
        return result;
    };

    result = sqlite3_exec(*db, "PRAGMA foreign_keys = ON;", NULL, NULL, &err_msg);
    if (result != SQLITE_OK) {
        fprintf(stderr, "Error enabling foreign key constraints: %s\n", sqlite3_errmsg(*db));
        return result;
    }

    return result;
} 

int build_ripple_database(void) {
    sqlite3 *db = NULL;
    char *err_msg = NULL;
    int result = 1; 

    result = db_open(&db);
    if (result != SQLITE_OK) {
        fprintf(stderr, "Error storing channel list\n");
        return result;
    }

    result = create_channel_table(db, &err_msg);
    if (result != SQLITE_OK) goto out;
    result = create_article_table(db, &err_msg);
    if (result != SQLITE_OK) goto out;

    out:
        return result;
}

int get_channel_id(sqlite3 *db, rss_channel *channel) {
    sqlite3_stmt *stmt = NULL;
    int result = 1; 
    int id = -1;
    char *select_statement_str = "SELECT id FROM channel WHERE link = ? LIMIT 1;";

    result = sqlite3_prepare_v2(db, select_statement_str, -1, &stmt, NULL);
    if (result != SQLITE_OK) {
        fprintf(stderr, "Error: could not get channel id (prep error) - %s\n", sqlite3_errmsg(db));
        goto cleanup;
    }

    result = sqlite3_bind_text(stmt, 1, channel->link, -1, NULL);
    if (result != SQLITE_OK) {
        fprintf(stderr, "Error: could not bind argument to SQL statement (binding error) - %s\n", sqlite3_errmsg(db));
        goto cleanup;
    }

    result = sqlite3_step(stmt);
    if (result != SQLITE_ROW) {
        fprintf(stderr, "Error: SQL statement to get channel id failed - %s\n", sqlite3_errmsg(db));
        goto cleanup;
    }

    id = sqlite3_column_int(stmt, 0);

    cleanup:
        if (stmt != NULL) sqlite3_finalize(stmt);
       
    return id;
    
}

int store_channel_list(size_t channel_count, rss_channel **channels) {
    sqlite3 *db = NULL;
    int result = 1;

    result = db_open(&db);
    if (result != SQLITE_OK) {
        fprintf(stderr, "Error storing channel list\n");
        return result;
    }
    
    for (size_t i = 0; i < channel_count; i++) {
        rss_channel *chan = channels[i];
        result = store_channel(db, chan);

        if (result != SQLITE_OK) {
            fprintf(stderr, "Error storing channel: %s\n", chan->title);
            continue;
        }

        int channel_id = get_channel_id(db, chan);
        if (channel_id < 1) {
            continue;
        }

        chan->id = channel_id;
        log_debug("Channel '%s' id: %i", chan->title, chan->id);
        for (size_t j = 0; j < chan->items->count; j++) {
            rss_item *item = chan->items->elements[j];
            result = store_article(db, item, chan);
            if (result != SQLITE_OK && result != SQLITE_CONSTRAINT_UNIQUE) {
                fprintf(stderr, "Error storing article: %s\n", item->title);
            }
        }
    }

    return (result != SQLITE_OK || result != SQLITE_CONSTRAINT_UNIQUE);
}

int get_channel_count(sqlite3 *db) {
    sqlite3_stmt *stmt = NULL; 
    int count = -1;
    const char *stmt_str = "SELECT COUNT(*) FROM channel;";
    if (sqlite3_prepare_v2(db, stmt_str, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Could not retrieve row count for the channel table: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}

int get_total_article_count(sqlite3 *db) {
    sqlite3_stmt *stmt = NULL; 
    int count = -1;
    const char *stmt_str = "SELECT COUNT(*) FROM article;";
    if (sqlite3_prepare_v2(db, stmt_str, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Could not retrieve row count for the article table: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}

int get_main_feed_articles(generic_list *article_list) {
    sqlite3_stmt *stmt = NULL;
    sqlite3 *db = NULL;
    int result = SQLITE_OK;    

    result = db_open(&db);
    if (result != SQLITE_OK) {
        fprintf(stderr, "Error connecting to database: %s\n", sqlite3_errmsg(db));
        return result;
    }; 

    const char *stmt_str = "SELECT a.id AS article_id, a.title AS article_title, a.author, a.description, a.unix_timestamp, a.link, c.id AS channel_id, c.title AS channel_title"
                            " FROM article AS a JOIN channel AS c"
                            " ON a.channel_id=c.id"
                            " ORDER BY a.unix_timestamp DESC;";

    if (sqlite3_prepare_v2(db, stmt_str, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error retrieving feed articles: %s\n", sqlite3_errmsg(db));
    }

    while ((result = sqlite3_step(stmt)) == SQLITE_ROW) {
        int argument_idx = 0;

        article_with_channel_name *article = malloc(sizeof(*article));
        if (!article) {
            fprintf(stderr, "Failed memory allocation for article\n");
            return 1;
        }

        rss_item *item = malloc(sizeof(*item));
        if (!item) {
            fprintf(stderr, "Failed memory allocation for item\n");
            return 1;
        }

        item->id = sqlite3_column_int(stmt, argument_idx++);

        const unsigned char *title = sqlite3_column_text(stmt, argument_idx++);
        item->title = title ? strdup((const char *)title) : NULL;
        
        const unsigned char *author = sqlite3_column_text(stmt, argument_idx++);
        item->author = author ? strdup((const char *)author) : NULL;         

        const unsigned char *description = sqlite3_column_text(stmt, argument_idx++);
        item->description = description ? strdup((const char *)description) : NULL; 

        item->unix_timestamp = sqlite3_column_int(stmt, argument_idx++);

        const unsigned char *link = sqlite3_column_text(stmt, argument_idx++);
        item->link = link ? strdup((const char *)link) : NULL; 

        item->channel_id = sqlite3_column_int(stmt, argument_idx++);

        const unsigned char *channel_title = sqlite3_column_text(stmt, argument_idx++);
        article->channel_name = channel_title ? strdup((const char *)channel_title) : NULL;

        article->item = item;
        list_append(article_list, article);
    }
 
    if(sqlite3_finalize(stmt) != SQLITE_OK) {
        fprintf(stderr, "Error finalizing statement for getting all articles.");
    }

    if (result != SQLITE_DONE) {
        fprintf(stderr, "Error converting statement to channel: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    return 0;
}

void free_article_with_channel_name(article_with_channel_name *article) {
    free_item(article->item);
    free(article->channel_name);
}