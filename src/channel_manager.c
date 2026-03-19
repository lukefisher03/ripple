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


int create_fetch_thread_pool(void) {
    fetch_parse_tp = thread_pool_create(TP_THREAD_COUNT, 100, _fetch_and_parse_channel, NULL);
    return fetch_parse_tp != NULL;
}

// We only refresh channels that the user looks at.
int refresh_channels(void) {
    time_t stale_channel_cutoff = time(NULL) - (REFRESH_PERIOD_HOURS * 3600);
    generic_list *stale_channels = list_init(); 

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
        fetch_parse_tp_enqueue(c->rss_link);
    }

cleanup:
    list_free(stale_channels);
    return 0;
}

int is_blank(char c) {
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
    if (!fetch_parse_tp) return 1;
    return thread_pool_add_work(link, fetch_parse_tp);
}

int fetch_parse_tp_busy(void) {
    if (!fetch_parse_tp) return 1;
    return thread_pool_busy(fetch_parse_tp);
}

void *_fetch_and_parse_channel(void *channel_link, void *arg) {
    (void) arg;
    char *link = (char *)channel_link;

    http_response *response = send_http_get(link);
    if (!response) {
        log_debug("Could not retrieve feed XML for %s. Skipping", link);
        return NULL;
    }
    rss_channel *new_channel = build_channel(response->body, response->body_size, link);
    free_http_response(response);
    if (!new_channel) return NULL;

    if (db_tp_enqueue(new_channel) != 0) {
        free_channel(new_channel);
    }

    return NULL;
}