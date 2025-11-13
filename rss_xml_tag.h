#ifndef RSS_XML_TAG_H
#define RSS_XML_TAG_H

#include "d_array.h"


typedef enum tag_type { TAG_OPEN, TAG_CLOSE, TAG_SELF_CLOSE, TAG_IGNORE, TAG_CDATA } tag_type;

#define ERR_INVALID_XML -1
#define ERR_MEMORY_ALLOCATION -2

struct rss_xml_attr {
    char    *name;
    char    *value;
};

struct rss_xml_tag {
    char                    *name;
    char                    *name_space;
    tag_type                type;
    struct rss_xml_attr     **attrs;
    size_t                  attr_count;
};

struct rss_xml_tag *build_rss_xml_tag(char *tag, size_t length);
void print_rss_xml_tag(struct rss_xml_tag *t);
void rss_xml_tag_free(struct rss_xml_tag *tag);
void rss_xml_attr_free(struct rss_xml_attr *attr);

// Static functions
static int _parse_tag_attrs(struct rss_xml_tag *t, char *tag_str, size_t length);
static void _rss_xml_clear_attrs(struct rss_xml_tag *tag);
static bool _is_xml_name_start_char(char c);
static bool _is_xml_special_char(char c);


#endif