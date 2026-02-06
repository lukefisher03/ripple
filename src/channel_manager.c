#include "channels_db/channel_db_api.h"
#include "parser/xml_rss.h"
#include "list.h"
#include "logger.h"
#include "http_get_rss_xml.h"
#include "utils.h"

#include <unistd.h>
#include <time.h>
#include <string.h>

#define REFRESH_PERIOD_HOURS 10

// We only refresh channels that the user looks at.
int refresh_channels(void) {
    time_t stale_channel_cutoff = time(NULL) - (REFRESH_PERIOD_HOURS);
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
        char *xml_rss = get_feed_xml(c->rss_link, &size);
        list_append(new_channels, build_channel(xml_rss, size, c->rss_link));
    }

    store_channel_list(new_channels->count, (rss_channel**)new_channels->elements);

cleanup:
    // Free the stale channels
    for (size_t i = 0; i < stale_channels->count; i++) {
        free_channel(new_channels->elements[i]);
        free_channel(stale_channels->elements[i]);
    }
    list_free(stale_channels);
    list_free(new_channels);
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
        for (; is_blank(feeds_file[i]) && i < length; i++);

        // Read characters
        start = i;
        for (; !is_blank(feeds_file[i]) && i < length; i++);
        link_len = i - start;
        char *new_channel_link = calloc(link_len + 1, sizeof(char));
        if (!new_channel_link) return 1;
        memcpy(new_channel_link, feeds_file + start, link_len);
        new_channel_link[link_len] = '\0';
        log_debug("New channel: %s", new_channel_link);
        list_append(list, new_channel_link);
    }

    return 0;
}

int store_new_channels(char **links, size_t link_count) {
    log_debug("STORING NEW CHANNELS!");
    int result = 0;
    generic_list *new_channels = list_init();

    for (size_t i = 0; i < link_count; i++) {
        char *link = links[i];
        size_t rss_size = 0;
        char *feed_xml = get_feed_xml(link, &rss_size);
        rss_channel *new_channel = build_channel(feed_xml, rss_size, link);
        if (!new_channel) {
            log_debug("Failed to build new channel from link: %s", link);
            free(feed_xml);
            result = 1;
            goto cleanup;
        }
        list_append(new_channels, new_channel);
    }

    result = store_channel_list(new_channels->count, (rss_channel**)new_channels->elements);
    if (result != 0) {
        log_debug("Failure when storing channels");
        goto cleanup;
    }

cleanup:
    for (size_t i = 0; i < new_channels->count; i++) {
        free_channel(new_channels->elements[i]);
    }
    list_free(new_channels);
    return 0;
}