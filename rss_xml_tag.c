#include "rss_xml_tag.h"

#include <stdio.h>
#include <string.h>

#include "utils.h"

struct rss_xml_tag *build_rss_xml_tag(char *tag_str, size_t length) {
    if (!length) {
        return NULL;
    }

    struct rss_xml_tag *t = malloc(sizeof(struct rss_xml_tag));
    if (!t) {
        return NULL;
    }
    
    t->type = TAG_IGNORE;
    t->attr_count = 0;
    t->name_space = NULL;
    t->name = NULL;
    t->attrs = NULL;
        
    // Determine the tag type first.
    if (tag_str[1] == '/') {
        t->type = TAG_CLOSE;
    } else if (tag_str[length - 2] == '/') {
        t->type = TAG_SELF_CLOSE;
    } else if (tag_str[1] == '?') {
        t->type = TAG_IGNORE;
    } else if (tag_str[1] == '!') {
        printf("Encountered CDATA\n");
        rss_xml_tag_free(t);
        return NULL;   
    }else {
        t->type = TAG_OPEN;
    }
    
    // The offset is the index of the very first charactrer of the 
    // full identifier.
    size_t identifier_length = 0;
    size_t name_space_length = 0;
    size_t index;
    for(index = 0; index < length && _is_xml_special_char(tag_str[index]); index++);
    
    size_t offset = index; // Skip starting characters at the beginning
    if (!_is_xml_name_start_char(tag_str[offset])) {
        rss_xml_tag_free(t); 
        return NULL;
    }

    for (; index < length && tag_str[index] != ' ' && tag_str[index] != '>'; index++) {
        if (tag_str[index] == ':') {
            if (t->name_space) {
                rss_xml_tag_free(t);
                fprintf(stderr, "XML elements can only belong to a single namespace!\n");
                return NULL;
            }
            name_space_length = identifier_length + 1;
            t->name_space = strndup(tag_str + offset, identifier_length);
        }
        identifier_length++;
    }

    t->name = strndup(tag_str + offset + name_space_length, identifier_length - name_space_length);

    // Get all the attributes
    if (t->type != TAG_CLOSE) {
        int res = _parse_tag_attrs(t, tag_str, length);
        if (res < 0) {
            fprintf(stderr, "INVALID XML DETECTED. Error code: %i\n", res);
            rss_xml_tag_free(t);
            return NULL;
        }
    }

    return t;
}

static int _parse_tag_attrs(struct rss_xml_tag *t, char *tag_str, size_t length) {
    // Given a tag string, parse all the attributes in it.
    //
    // Common case example:
    // <tagName name="value" name2="value2">
    //
    // Edge case example:
    // <tagName                  name        =    "value"       >

    size_t index = 0;
    
    // Do a first pass through the string to collect some data.
    // Collect the number of attributes by counting the number of equals signs
    // except for the ones within attribute value strings. 
    // And then also place the index pointer past the name of the tag to not 
    // mistake it for an attribute.

    bool within_quotes = false;
    bool found_whitespace = false;
    size_t expected_attribute_count = 0;
    
    size_t i;
    for (i = 0; i < length && (tag_str[i] == '<' || tag_str[i] == '?' || tag_str[i] == '/'); i++);
    for (; i < length; i++) {
        if (tag_str[i] == '"' || tag_str[i] == '\'') {
            if (within_quotes) within_quotes = false;
            else within_quotes = true;
        }
        if (tag_str[i] == '=' && !within_quotes) expected_attribute_count++;

        if (!found_whitespace) index = i;
        if (tag_str[i] == ' ' || tag_str[i] == '\n') {
            found_whitespace = true;
        }
    }

    if (!found_whitespace) return 0; // There are no attributes

    t->attr_count = 0;
    t->attrs = calloc(expected_attribute_count, sizeof(struct rss_xml_attr));
    if (!t->attrs) {
        return ERR_MEMORY_ALLOCATION;
    }

    for (; index < length; index++) {
        // Search for the beginning of an attribute. Skip these characters since they'll never
        // be the first character of an attribute.
        if (tag_str[index] == ' ' || tag_str[index] == '\n') continue;
        else if (_is_xml_special_char(tag_str[index])) break;
        else if (!_is_xml_name_start_char(tag_str[index])) return ERR_INVALID_XML;
        else {
            // Attribute found
            size_t start = index;
            size_t end;

            struct rss_xml_attr *attr = malloc(sizeof(struct rss_xml_attr));
            if (!attr) {
                free(t->attrs);
                return ERR_MEMORY_ALLOCATION;
            }
            
            // Search for a space or = which indicate the end of the attribute name string.
            for (end = start; end < length && tag_str[end] != '=' && tag_str[end] != ' '; end++);
            attr->name = strndup(tag_str + start, end - start);

            // Iterate until we find the first double quote or single quote. This indicates
            // the beginning of the value portion of the attribute.
            for(; end < length && tag_str[end] != '"' && tag_str[end] != '\''; end++);
            start = end + 1;
            end++;  // Increment here so we skip the starting quote and are able to find 
                    // the terminating quote

            // Search for terminating quote.
            for(;end < length && tag_str[end] != '"' && tag_str[end] != '\''; end++);
            attr->value = strndup(tag_str + start, end - start);

            t->attrs[t->attr_count++] = attr;
            index = end;
        } 
    }
    
    if (expected_attribute_count != t->attr_count) {
        // Something went wrong
        fprintf(stderr, "Could not parse XML tag: %s\n", tag_str);
        _rss_xml_clear_attrs(t);
        return ERR_INVALID_XML;
    }

    return t->attr_count;
}

void print_rss_xml_tag(struct rss_xml_tag *tag) {
    if (!tag) {
        return;
    }
    printf("Tag name: '%s'\n", tag->name);
    printf("Tag namespace: '%s'\n", tag->name_space);
    switch (tag->type) {
        case TAG_CLOSE:
            printf("Type: close\n");
            break;
        case TAG_OPEN:
            printf("Type: open\n");
            break;
        case TAG_IGNORE:
            printf("Type: ignore\n");
            break;
        case TAG_SELF_CLOSE:
            printf("Type: self-close\n");
            break;
        default:
            printf("Type: UNKNOWN");
            break;
    }
    if (tag->attr_count > 0) {
        printf("Attributes:\n");   
        for (size_t i = 0; i < tag->attr_count; i++) {
            struct rss_xml_attr *a = tag->attrs[i];
            printf("\t%s='%s'\n", a->name, a->value);
        }
    }
    printf("\n");
}

static bool _is_xml_name_start_char(char c) {
    if (alpha_test(c)) return true;

    switch (c) {
        case '-':
        case '_':
        case ':':
            return true;
        default:
            return false;
    }
}

static bool _is_xml_special_char(char c) {
    switch (c) {
        case '>':
        case '<':
        case '/':
        case '[':
        case '?':
        case '!':
        case ']':
            return true;
        default:
            return false;
    }
}


static void _rss_xml_clear_attrs(struct rss_xml_tag *tag) {
    if (!tag) return;

    for (size_t i = 0; i < tag->attr_count; i++) {
        rss_xml_attr_free(tag->attrs[i]);
    }

    tag->attrs = NULL;
    tag->attr_count = 0;
}

void rss_xml_tag_free(struct rss_xml_tag *tag) {
    if (!tag) return;

    _rss_xml_clear_attrs(tag);
    free(tag->name);
    free(tag->attrs);
    free(tag);
}

void rss_xml_attr_free(struct rss_xml_attr *attr) {
    if (!attr) return;

    free(attr->name);
    free(attr->value);
    free(attr);
}
