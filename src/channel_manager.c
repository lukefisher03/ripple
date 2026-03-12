#include "channel_manager.h"
#include "channels_db/channel_db_api.h"
#include "parser/xml_rss.h"
#include "list.h"
#include "logger.h"
#include "http_get_rss_xml.h"
#include "utils.h"
#include "thread_pool.h"

#include <unistd.h>
#include <time.h>
#include <string.h>

#define TP_THREAD_COUNT 5

static thread_pool *fetch_parse_tp = NULL;

void *fetch_and_parse_channel(void *channel_link, void *arg);

int create_thread_pools(void) {
    fetch_parse_tp = thread_pool_create(TP_THREAD_COUNT, 100, fetch_and_parse_channel, NULL);
    return fetch_parse_tp != NULL;
}

// We only refresh channels that the user looks at.
int refresh_channels(void) {
    time_t stale_channel_cutoff = time(NULL) - (REFRESH_PERIOD_HOURS * 3600);
    generic_list *stale_channels = list_init(); 
    generic_list *new_channels = list_init();

    if (!stale_channels) {
        log_debug("Failed to allocate new memory for refreshing channels");
        abort();
    }

    get_stale_channels(stale_channel_cutoff, stale_channels);

    if (list_is_empty(stale_channels)) {
        log_debug("No stale channels!");
        goto cleanup;
    }

    for (size_t i = 0; i < stale_channels->count; i++) {
        rss_channel *c = stale_channels->elements[i];
        delete_channel(c->id);
        size_t size = 0;
        fetch_parse_tp_enqueue(c->rss_link);
    }

cleanup:
    list_free(stale_channels);
    return 0;
}

bool is_blank(char c) {
    return c == '\n' || c == ' ' || c == '\t';
}

int get_new_channel_links(const char *feeds_file, size_t length, generic_list *list) {
    if (!feeds_file) return 1;
    size_t start = 0;
    size_t link_len = 0;

    for(size_t i = 0; i < length; i++) {
        // Skip whitespace
        for (; i < length && is_blank(feeds_file[i]); i++);
        if ((i == 0 || is_blank(feeds_file[i-1])) && feeds_file[i] == '#') {
            // Skip comments
            for (; i < length && feeds_file[i] != '\n'; i++);
            continue;
        }

        // Read characters
        start = i;
        for (; i < length && !is_blank(feeds_file[i]); i++);
        link_len = i - start;
        char *new_channel_link = calloc(link_len + 1, sizeof(char));
        
        if (!new_channel_link) return 1;

        memcpy(new_channel_link, feeds_file + start, link_len);

        new_channel_link[link_len] = '\0';
        log_debug("New channel: %s", new_channel_link);
        list_append(list, new_channel_link);
    }

    return list->count;
}

int fetch_parse_tp_enqueue(char *link) {
    return thread_pool_add_work(link, fetch_parse_tp);
}


void *fetch_and_parse_channel(void *channel_link, void *arg) {
    (void) arg;
    char *link = (char *)channel_link;


    size_t rss_size = 0;
    char *feed_xml = get_feed_xml(link, &rss_size);
    if (!feed_xml) {
        log_debug("Could not retrieve feed XML for %s. Skipping", link);
    }
    rss_channel *new_channel = build_channel(feed_xml, rss_size, link);
    free(feed_xml);
    if (!new_channel) {
        log_debug("Failed to build new channel from link: %s", link);
    } else {
        log_debug("BUILDING CHANNEL: %s", new_channel->title);
    }
   
    if (db_tp_enqueue(new_channel) != 0) {
        free_channel(new_channel);
    }

    return NULL;
}