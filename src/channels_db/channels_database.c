#include "channels_database.h"

#include <sqlite3.h>
#include <stdio.h>

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

    const char *insert_article_sql = "INSERT INTO article(title, author, description, link, channel_id, unix_timestamp) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt = NULL;
    int result;

    result = sqlite3_prepare_v2(db, insert_article_sql, -1, &stmt, NULL);
    if (result != SQLITE_OK) {
        fprintf(stderr, "Error storing article (during prep): %s\n", sqlite3_errmsg(db));
        goto cleanup;
    }

    int argument_idx = 1;

    result = sqlite3_bind_text(stmt, argument_idx++, item->title, -1, SQLITE_TRANSIENT);
    if (result != SQLITE_OK) goto bind_error;
    result = sqlite3_bind_text(stmt, argument_idx++, item->author, -1, SQLITE_TRANSIENT);
    if (result != SQLITE_OK) goto bind_error;
    result = sqlite3_bind_text(stmt, argument_idx++, item->description, -1, SQLITE_TRANSIENT);
    if (result != SQLITE_OK) goto bind_error;
    result = sqlite3_bind_text(stmt, argument_idx++, item->link, -1, SQLITE_TRANSIENT);
    if (result != SQLITE_OK) goto bind_error;
    result = sqlite3_bind_int(stmt, argument_idx++, channel->id);
    if (result != SQLITE_OK) goto bind_error;
    result = sqlite3_bind_int(stmt, argument_idx++, item->unix_timestamp);
    if (result != SQLITE_OK) goto bind_error;

    bind_error:
        if (result != SQLITE_OK) {
            fprintf(stderr, "Error storing article (during argument binding): %s\n", sqlite3_errmsg(db));
            goto cleanup; 
        }

    if ((result = sqlite3_step(stmt)) != SQLITE_DONE) {
        fprintf(stderr, "Error storing article (during sqlite3_step): %s\n", sqlite3_errmsg(db));
        goto cleanup;
    }

    result = SQLITE_OK;

    cleanup:
        if (stmt != NULL) sqlite3_finalize(stmt);

    return (result == SQLITE_OK) ? 0 : 1;
}

int store_channel(sqlite3 *db, const rss_channel *channel) {
    if (!channel || !db) {
        return 1;
    }

    char *insert_channel_sql = "INSERT INTO channel(title, description, language, link) VALUES (?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    int result;

    result = sqlite3_prepare_v2(db, insert_channel_sql, -1, &stmt, NULL);
    if (result != SQLITE_OK) {
        fprintf(stderr, "Error storing channel (during prep): %s\n", sqlite3_errmsg(db));
        goto cleanup;
    }

    int argument_idx = 1;
     
    result = sqlite3_bind_text(stmt, argument_idx++, channel->title, -1, NULL);
    if (result != SQLITE_OK) goto bind_error;
    result = sqlite3_bind_text(stmt, argument_idx++, channel->description, -1, NULL);
    if (result != SQLITE_OK) goto bind_error;
    result = sqlite3_bind_text(stmt, argument_idx++, channel->language, -1, NULL);
    if (result != SQLITE_OK) goto bind_error;
    result = sqlite3_bind_text(stmt, argument_idx++, channel->link, -1, NULL);
    if (result != SQLITE_OK) goto bind_error;

    bind_error:
        if (result != SQLITE_OK) {
            fprintf(stderr, "Error storing channel (during argument binding): %s\n", sqlite3_errmsg(db));
            goto cleanup; 
        }
    
    result = sqlite3_step(stmt);

    if ((result = sqlite3_step(stmt)) == SQLITE_ROW) {
        fprintf(stderr, "Error storing channel (during sqlite3_step): %s\n", sqlite3_errmsg(db));
        goto cleanup;
    }
    if (result != SQLITE_DONE) {
        result = SQLITE_OK;
    }

    cleanup:
        if (stmt != NULL) sqlite3_finalize(stmt);

    return (result == SQLITE_OK) ? 0 : 1;
}

// int main(int argc, char *argv[]) {
//     sqlite3 *db = NULL;
//     int err = sqlite3_open("dev.db", &db);

//     if (err) {
//         fprintf(stderr, "Could not open DB\n");
//         return 1;
//     } else {
//         printf("Database opened\n");
//     }

//     char *err_msg = NULL;
//     int result;
//     result = sqlite3_exec(db, "PRAGMA foreign_keys = ON;", NULL, NULL, &err_msg);
//     printf("Result 0 %s\n", err_msg);
//     result = create_channel_table(db, &err_msg);
//     printf("Result 1 %i\n", result);
//     result = create_article_table(db, &err_msg);
//     printf("Result 2 %i\n", result);

//     rss_channel channel = {
//         .description = "Channel description",
//         .id = 1,
//         .items = NULL,
//         .language = "english",
//         .last_build_date = "last build date",
//         .link = "https://thisisachannellink.com",
//         .title = "Lukie Pookie Channel",
//     };


//     rss_item item = {
//         .author = "My author",
//         .description = "my description.",
//         .title = "This is the title",
//         .unix_timestamp = 1235929,
//         .link = "https://thisisalink.com",
//         .channel = &channel,
//     };

//     store_channel(db, &channel);
//     store_article(db, &item);
//     result = store_article(db, &item);
//     printf("RESULT: %i\n", result);

//     if (sqlite3_exec(db, "select * from article", &simple_callback, NULL, &err_msg) != SQLITE_OK) {
//         fprintf(stderr, "Error getting columns: %s", err_msg);
//     }

//     sqlite3_close(db);
//     printf("Database closed\n");
//     return 0;
// }