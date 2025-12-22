#include "xml_rss.h"
#include "../utils.h"
#include "../logger.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#define TRSS_OK 0
#define TRSS_ERR -1

typedef struct xml_entity {
    char    ch;
    char    *s;
} xml_entity;

static xml_entity xml_entities[] = {
    {.ch = '<', .s = "&lt;"},
    {.ch = '>', .s = "&gt;"},
    {.ch = '"', .s = "&quot;"},
    {.ch = '\'', .s = "&#x27;"},
    {.ch = '\'', .s = "&apos;"},

};

// ======== Forward declarations ======== //

static inline bool is_termination_char(char c);
static char *get_spacer(int width);
rss_container *container_init(enum container_type t);
void free_container(rss_container *c);

// ======== Build parse tree ======== //

bool read_tag(const char *str, size_t length, Tag *t) {
    // Given a string starting with `<`, extract the tag name and
    // return the length of the tag.

    if (!t) return false;
    if (str[0] != '<') return false;

    size_t i = 1;
    if (str[i] == '/') {
        i++;
    }
    size_t offset = i;
    for (; i < length && !is_termination_char(str[i]); i++);
    t->name = strndup(str+offset, i-offset);
    for (; i < length && str[i] != '>'; i++);

    if (i >= length) return false;

    if (str[1] == '/') {
        t->tag_type = TAG_CLOSE;
    } else if (str[i - 1] == '/') {
        t->tag_type = TAG_SELF_CLOSE;
    } else {
        t->tag_type = TAG_OPEN;
    }

    t->total_length = i+1;
    return true;
}

xml_entity *replace_entity(const char *str) {
    size_t entity_count = sizeof(xml_entities) / sizeof(xml_entities[0]);
    for (size_t i = 0; i < entity_count; i++) {
        if (sstartswith(xml_entities[i].s, str, strlen(xml_entities[i].s))) {
            log_debug("Found match %s", xml_entities[i].s);
            return &xml_entities[i];
        }
    }
    // Return a space by default if the entity replacement is not found
    return NULL; 
}

ssize_t accumulate_text(const char *str, size_t length, rss_node *new_node) {
    // Create a text node containing all the contents until a closing tag is 
    // reached. 
    if (!new_node) {
        printf("EXITED DUE TO NOT HAVING NEW NODE!\n");
        return TRSS_ERR;
    }

    ssize_t i = 0;
    size_t total_length = 0;
    bool entity_replacement_enabled = true;
    // Detect the terminating part of the content
    const char *end_str = "<";
    if (sstartswith("<![CDATA[", str, length)) {
        i += 9;
        end_str = "]]>";
        total_length += 3;
        entity_replacement_enabled = false;
    }

    // Get the total length of the string
    size_t j = i;
    for (; j < length && !sstartswith(end_str, str+j, length - j); j++);

    size_t content_length = 0;
    total_length += j;
    content_length = j - i;
    
    new_node->text = malloc(content_length + 1);
    size_t len = 0;
    const char *s = str + i;

    for (size_t k = 0; k < content_length; k++) {
        // Perform XML entity replacement
        if ((unsigned char)s[k] < 0x80) {
            char ch = s[k]; 
            if (entity_replacement_enabled && s[k] == '&') {
                xml_entity *entity = replace_entity(s + k);
                if (entity != NULL) {
                    ch = entity->ch;
                    k += strlen(entity->s) - 1; 
                } 
            } 
            new_node->text[len++] = ch; 
        }
    }

    new_node->text[len] = '\0';
    new_node->type = TEXT_NODE;
    return total_length;
}

ssize_t skip_comment(const char *str, size_t length) {
    // Given a string starting with `<!--`, skip to the termination of
    // the comment `-->` and return the number of characters the comment
    // is. 
    if (!str) return TRSS_ERR;

    ssize_t i = 0;
    for (; i < length && !sstartswith("-->", str + i, length - i); i++);
    // Skip over the comment termination sequence.
    return i + 4;
}

rss_node *construct_parse_tree(const char *xml, size_t length) {
    // Given a string of RSS XML construct a parse tree.

    rss_node *root = xml_node_init();
    generic_list *stack = list_init();
    list_append(stack, root);

    size_t i = 0;
    while (i < length) {
        if (xml[i] == '\n' || xml[i] == '\t' || xml[i] == ' ') {
            i++;
            continue;
        }

        const char *s = xml + i;
        size_t l = length - i;
        
        if (sstartswith("<", s, l) && !sstartswith("<!", s, l) && !sstartswith("<?", s, l)) {
            Tag new_tag;
            if (read_tag(s, l, &new_tag)) { 
                if (new_tag.tag_type == TAG_OPEN) {
                    rss_node *top = list_peek(stack); 

                    rss_node *node = xml_node_init(); 
                    node->xml.name = strndup(new_tag.name, strlen(new_tag.name));
                    free(new_tag.name);
                    list_append(top->xml.children, node);
                    list_append(stack, node);

                } else if (new_tag.tag_type == TAG_CLOSE) {
                    rss_node *v = list_pop(stack);
                }

                i += new_tag.total_length;
            }
        } else if (sstartswith("<!--", s, l)) {
            ssize_t comment_length = skip_comment(s, l);
            if (comment_length != TRSS_ERR) {
                i += comment_length;
            } else {
                // fprintf(stderr, "Error skipping comment...\n");
            }
        }else {
            rss_node *t_node = text_node_init();
            ssize_t text_length = accumulate_text(s, l, t_node);

            if (text_length < 1) {
                // fprintf(stderr, "Error processing text node!\n");
                i++;
                continue;
            } 

            i += text_length;
            rss_node *top = list_peek(stack);
            list_append(top->xml.children, t_node);
        }
    }

    root->type = ROOT_NODE;
    root->xml.name = strdup("ROOT");
    return root;
}

void print_parse_tree(const rss_node *root, int depth) {
    // Recursive function for printing out an indented version of a
    // parse tree 

    if (!root) {
        return;
    }

    char *spacer = get_spacer(depth);

    if (!spacer) {
        fprintf(stderr, "Memory allocation failed!!");
        return;
    }

    switch (root->type) {
        case TEXT_NODE:
            printf("\n%sText Node\n", spacer);
            printf("%sText Content: '%s'\n", spacer, root->text);
            break;
        case ROOT_NODE: // The root node is just an XML node with a special label
        case XML_NODE:
            printf("\n%sNode: %s\n", spacer, root->xml.name);
            printf("%sChildren count: %lu\n", spacer, root->xml.children->count);
            for (size_t i = 0; i < root->xml.children->count; i++) {
                print_parse_tree(root->xml.children->elements[i], depth+1);
            }
        default:
            break;
    }
    free(spacer);
}

// ======== Build channels ======== //

int process_node(rss_container *c, const rss_node *n) {
    // This function only expects XML nodes, not text or dummy nodes
    if (n->type != XML_NODE) return TRSS_ERR;
    if (!n->xml.children->count) return TRSS_ERR;

    rss_node *text_node = (rss_node *)(n->xml.children->elements[0]);
    if (text_node->type != TEXT_NODE) return TRSS_ERR; 

    const char *node_name = n->xml.name;
    if (c->type == ITEM) {
        rss_item *item = c->item;
        // Remember that two strings are equal if strcmp(s1, s2) == 0
        if (!strcmp(node_name, "guid")) { 
            item->guid = strdup(text_node->text);
        } else if (!strcmp(node_name, "title")) {
            item->title = strdup(text_node->text);
        } else if (!strcmp(node_name, "author")) {
            item->author = strdup(text_node->text);
        } else if (!strcmp(node_name, "link")) {
            item->link = strdup(text_node->text);
        } else if (!strcmp(node_name, "pubDate")) {
            item->pub_date_rfc822 = strdup(text_node->text);
            struct tm tm = {0};
            if (rfc_822_to_tm(item->pub_date_rfc822, &tm)) {
                strftime(item->pub_date_string, MAX_RSS_ITEM_DATE_LEN, "%a, %D", &tm);
                item->pub_date_unix = timegm(&tm);
            } else {
                log_debug("Could not parse publish date string: %s, skipping", item->pub_date_rfc822);
            }
            
        } else if (!strcmp(node_name, "description")) {
            item->description = strdup(text_node->text);
        } else {
            // printf("No place for %s in container type %i\n", node_name, c->type);
        }
    } else if (c->type == CHANNEL) {
        rss_channel *channel = c->channel;
        if (!strcmp(node_name, "title")) {
           channel->title = strdup(text_node->text);
        } else if (!strcmp(node_name, "lastBuildDate")) {
           channel->last_build_date = strdup(text_node->text);
        } else if (!strcmp(node_name, "link")) {
           channel->link = strdup(text_node->text);
        } else if (!strcmp(node_name, "description")) {
           channel->description = strdup(text_node->text);
        }  else if (!strcmp(node_name, "language")) {
           channel->language = strdup(text_node->text);
        } else {
            // printf("No place for %s in container type %i\n", node_name, c->type);
        }
    } else {
        fprintf(stderr, "CONTAINER NOT RECOGNIZED: %i\n", c->type);
        return TRSS_ERR;
    }

    return TRSS_OK;
}

bool build_channel(rss_channel *chan, rss_node *root_node) {
    // Perform iterative DFS to build a channel from a parse tree

    generic_list *container_stack = list_init();
    
    generic_list *dfs_stack = list_init();
    list_append(dfs_stack, root_node); 

    while (!list_is_empty(dfs_stack)) {
        rss_node *node = list_pop(dfs_stack);

        switch (node->type) {
            case ROOT_NODE:
                for (ssize_t i = node->xml.children->count - 1; i >= 0; i--) {
                    list_append(dfs_stack, node->xml.children->elements[i]);
                }
                break;
            case DUMMY:
                // If we encounter a dummy node, we know that we just finished
                // iterating through a container's children
                free(list_pop(container_stack));
                break;
            case XML_NODE:
                if (!strcmp(node->xml.name,"item")) {
                    rss_container *new_item = container_init(ITEM);
                    new_item->item->channel = chan;
                    rss_container *parent = list_peek(container_stack);
                    list_append(chan->items, new_item->item);
                    list_append(container_stack, new_item);

                    // Append a dummy node so we know when the container has no more
                    // children
                    list_append(dfs_stack, dummy_node_init()); 
                    
                    // Append node's children backwards so that the stack has the 
                    // leftmost child on the top
                    for (ssize_t i = node->xml.children->count - 1; i >= 0; i--) {
                        list_append(dfs_stack, node->xml.children->elements[i]);
                    }
                } else if (!strcmp(node->xml.name, "channel")) {
                    // Create new container holding the channel
                    rss_container *root_container = malloc(sizeof(*root_container));
                    root_container->type = CHANNEL;
                    root_container->channel = chan;
                    list_append(container_stack, root_container);

                    for (ssize_t i = node->xml.children->count - 1; i >= 0; i--) {
                        rss_node *n = node->xml.children->elements[i];
                        list_append(dfs_stack, n);
                    }
                } else if (list_is_empty(container_stack)) {
                    // Append node's children backwards so that the stack has the 
                    // leftmost child on the top
                    for (ssize_t i = node->xml.children->count - 1; i >= 0; i--) {
                        rss_node *n = node->xml.children->elements[i];
                        list_append(dfs_stack, n);
                    }
                } else {
                    rss_container *c = list_peek(container_stack);
                    process_node(c, node);
                }
                break;
            default:
                break;
        }
    }

    // We're only cleaning up the containers
    for (size_t i = 0; i < container_stack->count; i++) {
        free(container_stack->elements[i]);
    }
    for (size_t i = 0; i < dfs_stack->count; i++) {
        free(dfs_stack->elements[i]);
    }

    list_free(container_stack);
    list_free(dfs_stack);
    return true;
}

rss_channel **load_channels(char *files[], size_t size) {
    rss_channel **channel_list = calloc(size, sizeof(*channel_list));
    if (!channel_list) {
        return NULL;
    }

    for (size_t i = 0; i < size; i++) {
        size_t size;
        char *rss = file_to_string(files[i], &size);
        rss_node *tree = construct_parse_tree(rss, size);
        free(rss);
        channel_list[i] = channel_init();
        build_channel(channel_list[i], tree);
        free_tree(tree);
    }

    return channel_list;
}

// ======== Initializers ======== //

rss_item *item_init(void) {
    rss_item *new_item = calloc(1, sizeof(*new_item));
    if (!new_item) {
        return NULL;
    }
    
    return new_item;
}

rss_channel *channel_init(void) {
    rss_channel *new_channel = calloc(1, sizeof(*new_channel));
    if (!new_channel) {
        return NULL;
    }

    new_channel->items = list_init();
    return new_channel;
}

rss_container *container_init(enum container_type t) {
    rss_container *new_container = malloc(sizeof(*new_container));
    
    if (!new_container) return NULL;
    new_container->type = t;

    switch (t) {
    case ITEM:
        new_container->item = item_init();
        break;
    case CHANNEL:
        new_container->channel = channel_init();
        break;
    default:
        break;
    }

    return new_container;
}

// ======== Utility functions ======== //

static char *get_spacer(int width) {
    // Build spacer to properly indent printing a tree

    char *spacer = calloc(width+1, sizeof(*spacer));
    if (!spacer) {
        return NULL;
    }
    memset(spacer, '\t', width);
    spacer[width] = '\0';
    return spacer;
}

static inline bool is_termination_char(char c) {
    return (c == ' ' || c == ':' || c == '/' || c == '>');
}

// ======== Cleanup functions ======== //

void free_item(rss_item *it) {
    free(it->title);
    free(it->description);
    free(it->author);
    free(it->guid);
    free(it->link);
    free(it->pub_date_rfc822);
    free(it);
}

void free_channel(rss_channel *c) {
    free(c->description);
    free(c->title);
    free(c->language);
    free(c->last_build_date);
    free(c->link);

    for (size_t i = 0; i < c->items->count; i++) {
        rss_item *it = c->items->elements[i];
        free_item(it);
    }
    free(c);
}

void free_container(rss_container *c) {
    switch (c->type)
    {
    case CHANNEL:
        free_channel(c->channel);
        break;
    case ITEM:
        free_item(c->item);
        break;
    default:
        break;
    }
}