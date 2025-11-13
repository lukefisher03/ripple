#include "utils.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rss_xml_tag.h"

#define MAX_FILE_SIZE 5000000 // Maximum RSS feed page size is 5MB
#define CHUNK_SIZE 4096
#define MAX_TAG_SIZE 4096


// An article must have at the very least a title OR a description.
struct article {
    char    *title;
    int     pub_date;
    char    *author;
    char    *description;
    char    *guid;
    char    *text_content;
};

struct rss_feed {
    int             last_updated;
    int             article_count;
    char            *name;
    char            *source_url;
    struct article  **items;
};

struct rss_feed *build_feed(char *xml, size_t size) {
    struct rss_feed *feed = calloc(1, sizeof(struct rss_feed));

    int tag_count = 0;
    size_t i = 0;

    while (i < 5000) {
        char c = xml[i];
        if (c == '<') {
            size_t tag_start = i;
            size_t tag_length = 0;

            for (; xml[tag_start + tag_length] != '>'; tag_length++);
            tag_length++;
            char *tag_str = strndup(xml + tag_start, tag_length);
            printf("Raw tag: %s\n", tag_str);
            struct rss_xml_tag *t = build_rss_xml_tag(tag_str, tag_length);
            tag_count++;
            free(tag_str);
            print_rss_xml_tag(t);
        }
        i++;
    }

    feed->article_count = tag_count;
    return feed;
}

int main(int argc, char *argv[]) {
    size_t size;
    char *rss = file_to_string("smart_less.xml", &size);
    struct rss_feed *feed = build_feed(rss, size);
    printf("Feed article count: %i\n", feed->article_count);
    free(rss);
}