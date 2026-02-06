#include "channel_db_api.h"
#include "../logger.h"

#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifdef __APPLE__ 
    HOST_OS host_os = DARWIN;
#elif __linux__
    HOST_OS host_os = LINUX;
#else
    HOST_OS host_os = UNSUPPORTED;
#endif

#define MAX_DB_PATH_LEN 128 

char DB_PATH[MAX_DB_PATH_LEN];

void get_db_path(void) {
    char *user = getlogin();
    char db_directory[MAX_DB_PATH_LEN];

    switch (host_os) {
        case DARWIN:
            if (snprintf(db_directory, MAX_DB_PATH_LEN, "/Users/%s/Library/Application Support/ripple_rss", user) >= MAX_DB_PATH_LEN) {
                log_debug("Could not access database! Path is too long: %s", db_directory);
                abort();
            }
            break;
        case LINUX: 
            if (snprintf(db_directory, MAX_DB_PATH_LEN, "/home/%s/.local/ripple_rss", user) >= MAX_DB_PATH_LEN) {
                log_debug("Could not access database! Path is too long: %s", db_directory);
                abort();
            }
            break;
        case UNSUPPORTED:
            log_debug("Unsupported platform!");
            break;
        default:
            break;
    }
    
    if (mkdir(db_directory, 0777) == -1) {
        if (errno != EEXIST) {
            log_debug("Error: Failed to make ripple directory at %s", db_directory);
            abort();
        }
    }

    if (snprintf(DB_PATH, MAX_DB_PATH_LEN, "%s/ripple.db", db_directory) >= MAX_DB_PATH_LEN) {
        log_debug("Error: Failed to build path! %s", db_directory);
        abort();
    } 
}

// TODO: Refactor error handling and database closing here.
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
    log_debug("Creating database tables");
    sqlite3 *db = NULL;
    char *err_msg = NULL;
    int result = 1; 

    result = db_open(&db);
    if (result != SQLITE_OK) {
        log_debug("Error storing channel list\n");
        goto cleanup;
    }

    result = create_channel_table(db, &err_msg);
    if (result != SQLITE_OK) goto cleanup;
    result = create_article_table(db, &err_msg);
    if (result != SQLITE_OK) goto cleanup;

    cleanup:
        if (result != SQLITE_OK) {
            log_debug("Failed to create database tables, %s %i", sqlite3_errmsg(db), result);
        }
        sqlite3_close(db);
        return result;
}

int get_channel_id(sqlite3 *db, const rss_channel *channel) {
    sqlite3_stmt *stmt = NULL;
    int result = 1; 
    int id = -1;
    char *select_statement_str = "SELECT id FROM channel WHERE link = ? LIMIT 1;";

    result = sqlite3_prepare_v2(db, select_statement_str, -1, &stmt, NULL);
    if (result != SQLITE_OK) goto cleanup;

    result = sqlite3_bind_text(stmt, 1, channel->link, -1, NULL);
    if (result != SQLITE_OK) goto cleanup;

    result = sqlite3_step(stmt);
    if (result != SQLITE_ROW) goto cleanup;
    result = SQLITE_OK;

    id = sqlite3_column_int(stmt, 0);

    cleanup:
        if (result != SQLITE_DONE && result != SQLITE_OK) {
            log_debug("DB Error getting channel id, %s", sqlite3_errmsg(db));
        } 

        sqlite3_finalize(stmt);
    return id;
}

int store_channel_list(size_t channel_count, rss_channel **channels) {
    sqlite3 *db = NULL;
    int result = 1;

    result = db_open(&db);
    if (result != SQLITE_OK) goto cleanup;
    
    for (size_t i = 0; i < channel_count; i++) {
        rss_channel *chan = channels[i];
        result = store_channel(db, chan);

        if (result != SQLITE_OK) {
            continue;
        }

        int channel_id = get_channel_id(db, chan);
        if (channel_id < 1) {
            continue;
        }

        chan->id = channel_id;
        for (size_t j = 0; j < chan->items->count; j++) {
            rss_item *item = chan->items->elements[j];
            result = store_article(db, item, chan);
            // Should processing stop here?
            if (result != SQLITE_OK && sqlite3_extended_errcode(db) != SQLITE_CONSTRAINT_UNIQUE) {
                log_debug("Failed to store article: %s, skipping", item->title);
            }
        }
    }
cleanup:
    sqlite3_close(db);
    return result;
}
        
int get_channel_list(generic_list *article_list) {
    sqlite3_stmt *stmt = NULL;
    sqlite3 *db = NULL;
    int result = 1;

    result = db_open(&db);
    if (result != SQLITE_OK) goto cleanup;

    const char *stmt_str = "SELECT * FROM channel ORDER BY last_updated DESC, title DESC;";

    result = sqlite3_prepare_v2(db, stmt_str, -1, &stmt, NULL);
    if (result != SQLITE_OK) goto cleanup;

    while ((result = sqlite3_step(stmt)) == SQLITE_ROW) {
        rss_channel *chan = channel_init();
        if (!chan) {
            log_debug("Failed memory allocation");
            goto cleanup;
        }
        
        read_channel_from_stmt(stmt, chan);
        list_append(article_list, chan);
    }

cleanup: 
    if (result != SQLITE_DONE && result != SQLITE_OK) {
        log_debug("DB Error getting channel list, %s", sqlite3_errmsg(db));
    } else {
        result = SQLITE_OK;
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return result;
}

int get_channel_article_count(const rss_channel *channel, int *out_count) {
    if (!channel || !out_count) {
        return -1;
    }
    sqlite3_stmt *stmt = NULL;
    sqlite3 *db = NULL;
    int result = -1;

    result = db_open(&db);
    if (result != SQLITE_OK) goto cleanup;
    
    int id = get_channel_id(db, channel);
    if (id < 1) {
        goto cleanup;
    }

    const char *stmt_str = "SELECT COUNT(*) FROM article WHERE channel_id=?;";
    result = sqlite3_prepare_v2(db, stmt_str, -1, &stmt, NULL);
    if (result != SQLITE_OK) goto cleanup;
    

    result = sqlite3_bind_int(stmt, 1, id);
    if (result != SQLITE_OK) goto cleanup;

    if ((result = sqlite3_step(stmt)) == SQLITE_ROW) {
        int val = sqlite3_column_int(stmt, 0);
        *out_count = val;
    } else {
        goto cleanup;
    }
    result = SQLITE_OK;

cleanup:
    if (result != SQLITE_DONE && result != SQLITE_OK) {
        log_debug("DB Error getting channel article count, %s", sqlite3_errmsg(db));
    } else {
        result = SQLITE_OK;
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return result;
}

int get_main_feed_articles(generic_list *article_list) {
    sqlite3_stmt *stmt = NULL;
    sqlite3 *db = NULL;
    int result = 1;    

    result = db_open(&db);
    if (result != SQLITE_OK) goto cleanup;

    const char *stmt_str = "SELECT a.id AS article_id, a.title AS article_title, a.author, a.description, a.link, a.unix_timestamp, c.id AS channel_id, c.title AS channel_title"
                            " FROM article AS a JOIN channel AS c"
                            " ON a.channel_id=c.id"
                            " ORDER BY a.unix_timestamp DESC;";

    result = sqlite3_prepare_v2(db, stmt_str, -1, &stmt, NULL);
    if (result != SQLITE_OK) goto cleanup;

    while ((result = sqlite3_step(stmt)) == SQLITE_ROW) {
        int argument_idx = 0;

        article_with_channel_name *article = malloc(sizeof(*article));
        if (!article) {
            log_debug("Failed memory allocation");
            goto cleanup;
        }

        rss_item *item = item_init();
        if (!item) {
            log_debug("Failed memory allocation");
            goto cleanup;
        }

        argument_idx += read_article_from_stmt(stmt, item); 
        const unsigned char *channel_title = sqlite3_column_text(stmt, argument_idx++);
        article->channel_name = channel_title ? strdup((const char *)channel_title) : NULL;

        article->item = item;
        list_append(article_list, article);
    }

    if (result != SQLITE_DONE) goto cleanup;

cleanup: 
    if (result != SQLITE_DONE && result != SQLITE_OK) {
        log_debug("Error getting feed articles, %s", sqlite3_errmsg(db));
    } else {
        result = SQLITE_OK;
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return result;
}

int get_channel_articles(rss_channel *channel, generic_list *out_list) {
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;

    int result = 1;
    if ((result = db_open(&db)) != SQLITE_OK) goto cleanup;

    int channel_id = get_channel_id(db, channel);
    const char *stmt_str = "SELECT * FROM article WHERE channel_id=? ORDER BY unix_timestamp DESC;";
    result = sqlite3_prepare_v2(db, stmt_str, -1, &stmt, NULL);
    if (result != SQLITE_OK) goto cleanup;

    result = sqlite3_bind_int(stmt, 1, channel_id);
    if (result != SQLITE_OK) goto cleanup;

    while ((result = sqlite3_step(stmt)) == SQLITE_ROW) {
        rss_item *article = item_init();
        read_article_from_stmt(stmt, article);
        list_append(out_list, article);
    }
    if (result != SQLITE_DONE) goto cleanup;

cleanup:
    if (result != SQLITE_DONE && result != SQLITE_OK) {
        log_debug("Error getting channel articles, %s", sqlite3_errmsg(db));
    } else {
        result = SQLITE_OK;
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return result;
}

int get_channel(int channel_id, rss_channel *channel) {
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;

    int result = 1;
    if ((result = db_open(&db)) != SQLITE_OK) goto cleanup;

    const char *stmt_str = "SELECT * FROM channel WHERE id=?;";
    result = sqlite3_prepare_v2(db, stmt_str, -1, &stmt, NULL);
    if (result != SQLITE_OK) goto cleanup;

    result = sqlite3_bind_int(stmt, 1, channel_id);
    if (result != SQLITE_OK) goto cleanup;

    while ((result = sqlite3_step(stmt)) == SQLITE_ROW) {
        read_channel_from_stmt(stmt, channel);
    }

cleanup:
    if (result != SQLITE_DONE && result != SQLITE_OK) {
        log_debug("DB Error getting channel, %s", sqlite3_errmsg(db));
    } else {
        result = SQLITE_OK;
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return result;
}

int get_article(int article_id, rss_item *article) {
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;

    int result = 1;
    if ((result = db_open(&db)) != SQLITE_OK) goto cleanup;

    const char *stmt_str = "SELECT * FROM article WHERE id=?;";
    result = sqlite3_prepare_v2(db, stmt_str, -1, &stmt, NULL);
    if (result != SQLITE_OK) goto cleanup;

    result = sqlite3_bind_int(stmt, 1, article_id);
    if (result != SQLITE_OK) goto cleanup;

    while ((result = sqlite3_step(stmt)) == SQLITE_ROW) {
        read_article_from_stmt(stmt, article);
    }

cleanup:
    if (result != SQLITE_DONE && result != SQLITE_OK) {
        log_debug("DB Error getting article, %s", sqlite3_errmsg(db));
    } else {
        result = SQLITE_OK;
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return result;
}

int get_stale_channels(time_t cutoff, generic_list *out_list) {
    sqlite3 *db = NULL; 
    sqlite3_stmt *stmt = NULL;

    int result = 1;
    if ((result = db_open(&db)) != SQLITE_OK) goto cleanup;

    const char *stmt_str = "SELECT * FROM channel WHERE last_updated < ?;";
    result = sqlite3_prepare_v2(db, stmt_str, -1, &stmt, NULL);
    if (result != SQLITE_OK) goto cleanup;

    result = sqlite3_bind_int64(stmt, 1, (sqlite_int64)cutoff);
    if (result != SQLITE_OK) goto cleanup;

    while ((result = sqlite3_step(stmt) == SQLITE_ROW)) {
        rss_channel *new_channel = channel_init();
        if (!new_channel) {
            result = SQLITE_ERROR;
            log_debug("Failed to allocate new channel");
            goto cleanup;
        }

        read_channel_from_stmt(stmt, new_channel);
        list_append(out_list, new_channel);
    }

cleanup:
    if (result != SQLITE_OK && result != SQLITE_DONE) {
        log_debug("Error getting stale channels, %s", sqlite3_errmsg(db));
    } else {
        result = SQLITE_OK;
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return result;
}

int delete_channel(int channel_id) {
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;

    int result = 1;
    if ((result = db_open(&db)) != SQLITE_OK) goto cleanup;

    const char *stmt_str = "DELETE FROM channel WHERE id=?;";

    result = sqlite3_prepare_v2(db, stmt_str, -1, &stmt, NULL);
    if (result != SQLITE_OK) goto cleanup;

    result = sqlite3_bind_int(stmt, 1, channel_id);
    if (result != SQLITE_OK) goto cleanup;

    if ((result = sqlite3_step(stmt)) != SQLITE_DONE) goto cleanup;

cleanup:
    if (result != SQLITE_DONE && result != SQLITE_OK) {
        log_debug("DB Error deleting channel, %s", sqlite3_errmsg(db));
    } else {
        result = SQLITE_OK;
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return result; 
}

void free_article_with_channel_name(article_with_channel_name *article) {
    free_item(article->item);
    free(article->channel_name);
    free(article);
}