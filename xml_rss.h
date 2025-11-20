#ifndef XML_RSS_H
#define XML_RSS_H

#include <stdlib.h>
#include <sys/types.h>
#include <stdbool.h>

#define MAX_FILE_SIZE 5000000 // Maximum RSS feed page size is 5MB
#define CHUNK_SIZE 4096
#define MAX_TAG_SIZE 4096

enum tag_type {
    TAG_OPEN,
    TAG_CLOSE,
    TAG_SELF_CLOSE,
    CDATA_BLOCK,
    COMMENT_BLOCK,
};

struct tag {
    enum tag_type   type;
    char            *name; // RSS Tag names should be much smaller than this
};

struct article {
    char    *title;
    int     pub_date;
    char    *author;
    char    *description;
    char    *guid;
    char    *text_content;
};

struct rss_feed {
    int                     last_updated;
    int                     article_count;
    char                    *name;
    char                    *source_url;
    struct dynamic_stack    *articles;
};

/* Construct an RSS feed from its XML */
struct rss_feed *build_feed(char *xml, size_t size);

/* Parse an XML tag and fill out a tag struct. 
   Return the number of characters the tag is. */
ssize_t parse_tag(char *s, size_t length, struct tag *new_tag);

/* Starting from a certain point in a string, skip 
   until you find the next tag termination character. 
   Will do error handling to make sure another tag is not 
   opened up inside the current tag. */
ssize_t skip_tag(char *s, size_t length);

/* Initialize the `rss_feed` struct */
struct rss_feed *rss_feed_init(void);

/* Initialize the `article` struct */
struct article *article_init(void);

/* Given a tag and the parser element stack, do the appropriate 
   operation with the given tag. Return true if the tag was correctly 
   categorized and the right operation performed. Return false 
   otherwise */
bool decode_tag(struct tag *new_tag, struct dynamic_stack *stack);

/* Free the `tag` struct */
void free_tag(struct tag *t);

#endif