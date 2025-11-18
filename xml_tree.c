#include "utils.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rss_xml_tag.h"
#include "stack.h"
#include "dynamic_string.h"

#define MAX_FILE_SIZE 5000000 // Maximum RSS feed page size is 5MB
#define CHUNK_SIZE 4096
#define MAX_TAG_SIZE 4096


// An article must have at the very least a title OR a description.
struct article {
    struct string_d     *title;
    int                 pub_date;
    struct string_d     *author;
    struct string_d     *description;
    struct string_d     *guid;
    struct string_d     *text_content;
};

struct rss_feed {
    int                     last_updated;
    int                     article_count;
    struct string_d         *name;
    struct string_d         *source_url;
    struct dynamic_stack    *articles;
};

enum PARSER_STATES {
    META_STATE,
    CHANNEL_STATE,
    ITEM_STATE,
};

enum RSS_TAGS {
    DESCRIPTION_TAG,
    LINK_TAG,
    PUB_DATE_TAG,
    CATEGORY_TAG,
    TITLE_TAG,
    GUID_TAG,
    NO_TAG,
};

struct rss_feed *rss_feed_init(void) {
    struct rss_feed *feed = calloc(1, sizeof(struct rss_feed));

    if (!feed) {
        return NULL;
    }

    feed->articles = stack_init();

    if (!feed->articles) {
        free(feed);
        return NULL;
    }

    return feed;
}

struct article *article_init(void) {
    struct article *art = calloc(1, sizeof(struct article));

    if (!art) {
        return NULL;
    }

    art->title = string_d_init("No title!");

    return art;
}

struct article *article_clone(struct article *a) {
    struct article *new_article = article_init();

    if (!new_article) {
        fprintf(stderr, "Failed to allocate new article, memory error!\n");
        return NULL;
    }

    new_article->author = string_d_clone(a->author);
    new_article->description = string_d_clone(a->description);
    new_article->guid = string_d_clone(a->guid);
    new_article->pub_date = a->pub_date;
    new_article->text_content = string_d_clone(a->text_content);
    new_article->title = string_d_clone(a->title);

    return new_article;
}

void reset_article(struct article *a) {
    a->pub_date = 0;
    string_d_reset(a->author);
    string_d_reset(a->title);
    string_d_reset(a->guid);
    string_d_reset(a->text_content);
    string_d_reset(a->description);
}  

void article_free(struct article *a) {
    string_d_free(a->author);
    string_d_free(a->description);
    string_d_free(a->title);
    string_d_free(a->guid);
    string_d_free(a->text_content);
    free(a);
}

struct rss_feed *build_feed(char *xml, size_t size) {
    struct rss_feed *feed = rss_feed_init();
    int tag_count = 0;
    size_t i = 0;
    
    struct article *latest_article = article_init();
    enum PARSER_STATES state = META_STATE;

    size_t text_content_start = 0;
    size_t text_content_end = 0;

    while (i < size) {
        char c = xml[i];
        if (c == '<' && xml[i+1] != '!' && xml[i+1] != '?') {
            size_t tag_start = i;
            size_t tag_length = 0;

            for (; xml[tag_start + tag_length] != '>'; tag_length++);
            tag_length++;
            char *tag_str = strndup(xml + tag_start, tag_length);

            struct rss_xml_tag *t = build_rss_xml_tag(tag_str, tag_length);
            
            if (t) {
                if (strcmp(t->name, "channel") == 0) {
                    if (t->type == TAG_OPEN) state = CHANNEL_STATE;
                    if (t->type == TAG_CLOSE) state = META_STATE;
                } else if (strcmp(t->name, "item") == 0) {
                    if (t->type == TAG_OPEN) state = ITEM_STATE;
                    if (t->type == TAG_CLOSE) {
                        state = CHANNEL_STATE; // Change state back to the channel state
                        stack_push(feed->articles, article_clone(latest_article));
                        reset_article(latest_article);
                        printf("Reset article object\n");
                    }
                }
            }

            tag_count++;
            printf("%s\n", tag_str);
            free(tag_str);
            // i += tag_length - 1;
        }
        
        // printf("%c",xml[c]);
        i++;
    }

    feed->article_count = tag_count;
    return feed;
}

int main(int argc, char *argv[]) {
    size_t size;
    char *rss = file_to_string("test/stack_overflow.xml", &size);
    struct rss_feed *feed = build_feed(rss, size);
    printf("Feed article count: %i\n", feed->article_count);
    free(rss);

    // struct string_d *s = string_d_init("Hello there");
    // printf("%s\n", s->str);
    // string_d_append(s, " my friend!");
    // printf("%s\n", s->str);
    // string_d_free(s);
}