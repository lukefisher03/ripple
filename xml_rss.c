#include "xml_rss.h"
#include "stack.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>

struct rss_feed *build_feed(char *xml, size_t size) {
    /* Build an RSS feed of articles */
    struct rss_feed *feed = rss_feed_init(); // Initialize our feed object
    struct article *current_article = article_init(); // This object will get reused to add new articles

    struct dynamic_stack *e_stack = stack_init(); // Element stack to keep track of what elements to add next

    size_t i = 0;
    bool cdata_mode = false;

    while (i < size) {
        if (prefixcmp("]]>", xml + i, size - i) && cdata_mode) {
            /* HANDLE CLOSING CDATA*/
            cdata_mode = false;
            i++;
        } else if (prefixcmp("<?", xml + i, size - i) || prefixcmp("<!-", xml + i, size - i)) {
            // Skip comments and random metadata
            ssize_t v = skip_tag(xml + i, size - i); 

            if (v > 0) {
                i += v;
            } else {
                fprintf(stderr, "ERROR: No closing `>` for tag at position %lu\n", i);
                break;
            }

        } else if (prefixcmp("<![CDATA", xml + i, size - i) && !cdata_mode) {
            cdata_mode = true;
        } else if (prefixcmp("<", xml + i, size - i) && !cdata_mode) {
            // Found new tag
            struct tag *new_tag = malloc(sizeof(struct tag));
            if (!new_tag) {
                fprintf(stderr, "Memory allocation error!\n");
                break;
            }
        
            size_t count;
            if ((count = parse_tag(xml + i, size - i, new_tag)) <= 0) {
                fprintf(stderr, "Error parsing tag at position %lu\n", i);
                break;
            }

            i += count + 1;

            if (!decode_tag(new_tag, e_stack)) {
                fprintf(stderr, "Could not decode tag: %s, skipping\n", new_tag->name);
            }
        } else {
            // ACCUMULATE TEXT BETWEEN
            printf("%c", xml[i]);
            i++;
        }
    }

    // Free the element stack.
    for (size_t i = 0; i < e_stack->count; i++) {
        free_tag((struct tag*)e_stack->elements[i]);
    }
    stack_free(e_stack);
    return NULL;
}

ssize_t skip_tag(char *s, size_t length) {
    /* Return the number of characters until the first `>` character. */
    for (size_t i = 1; i < length; i++) {
        if (s[i] == '<') {
            return -1;
        }
        if (s[i] == '>') {
            return i+1;
        }
    }
    // Return -1 if there is no tag termination character.
    return -1;
}

ssize_t parse_tag(char *s, size_t length, struct tag *new_tag) {
    if (length <= 0) return -1; 
    if (s[0] != '<') return -1; // Tags must begin with `<`

    ssize_t i = 1;
    new_tag->type = TAG_OPEN;

    if (s[i] == '/') {
        new_tag->type = TAG_CLOSE;
        i++;
    }
    
    ssize_t name_start = i;
    for (; s[i] != ' ' && s[i] != '>' && s[i] != '/' && s[i] != ':'; i++);
    if (i - name_start > 256) {
        // TODO: Make this a toggleable setting for max tag length
        printf("Encountered abnormally long tag name... proceeding forward anyways.");
    }

    new_tag->name = strndup(s + name_start, i - name_start);

    for (; s[i] != '>'; i++);

    if (s[i-1] == '/') {
        new_tag->type = TAG_SELF_CLOSE;
    }

    return i;
}

bool decode_tag(struct tag *new_tag, struct dynamic_stack *stack) {
    switch (new_tag->type) {
        case TAG_OPEN:
            stack_push(stack, new_tag);
            break;
        case TAG_CLOSE: {
            struct tag *top_tag = stack_pop(stack);
            // printf("Popped off: %s\n", top_tag_name);
            if (strcmp(new_tag->name, top_tag->name) != 0) {
                fprintf(stderr, "Found closing tag doesn't match opening tag!\nExpected: </%s>\nGot: </%s>\n", top_tag->name, new_tag->name);
                return NULL;
            }
            break;
        }
        case TAG_SELF_CLOSE: break;
        default:
            return false;
    }

    return true;
}

void free_tag(struct tag *t) {
    if (!t) return;
    free(t->name);
    free(t);
}

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

    art->title = NULL;
    art->description = NULL;
    art->author = NULL;
    art->guid = NULL;
    art->text_content = NULL;
    return art;
}