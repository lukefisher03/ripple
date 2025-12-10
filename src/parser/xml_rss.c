#include "xml_rss.h"
#include "../utils.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#define TRSS_OK 0
#define TRSS_ERR -1

// ======== Forward declarations ======== //

static inline bool is_termination_char(char c);
static char *get_spacer(int width);
struct container *container_init(enum CONTAINER_TYPE t);
void free_container(struct container *c);

// ======== Build parse tree ======== //

bool read_tag(const char *str, size_t length, struct tag *t) {
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

ssize_t accumulate_text(const char *str, size_t length, struct node *new_node) {
    // Create a text node containing all the contents until a closing tag is 
    // reached. 
    if (!new_node) {
        printf("EXITED DUE TO NOT HAVING NEW NODE!\n");
        return TRSS_ERR;
    }

    ssize_t i = 1;
    size_t offset = 0;
    size_t total_length = 0;
    if (sstartswith("<![CDATA[", str, length)) {
        for (; i < length && !sstartswith("]]>", str + i, length - i); i++);
        total_length = i+3;
        offset = 9;
    } else {
        for (; i < length && str[i] != '<'; i++); 
        total_length = i;
    }

    new_node->type = TEXT_NODE;
    // The offset ensures we don't copy `<![CDATA[` or `]]>` into the
    // string buffer.
    new_node->text = strndup(str + offset, i - offset);
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

struct node *construct_parse_tree(const char *xml, size_t length) {
    // Given a string of RSS XML construct a parse tree.

    struct node *root = xml_node_init();
    struct list *stack = list_init();
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
            struct tag new_tag;
            if (read_tag(s, l, &new_tag)) { 
                if (new_tag.tag_type == TAG_OPEN) {
                    struct node *top = list_peek(stack); 

                    struct node *node = xml_node_init(); 
                    node->xml.name = strndup(new_tag.name, strlen(new_tag.name));
                    free(new_tag.name);
                    list_append(top->xml.children, node);
                    list_append(stack, node);

                } else if (new_tag.tag_type == TAG_CLOSE) {
                    struct node *v = list_pop(stack);
                }

                i += new_tag.total_length;
            }
        } else if (sstartswith("<!--", s, l)) {
            ssize_t comment_length = skip_comment(s, l);
            if (comment_length != TRSS_ERR) {
                i += comment_length;
            } else {
                fprintf(stderr, "Error skipping comment...\n");
            }
        }else {
            struct node *t_node = text_node_init();
            ssize_t text_length = accumulate_text(s, l, t_node);

            if (text_length < 1) {
                fprintf(stderr, "Error processing text node!\n");
                i++;
                continue;
            } 

            i += text_length;
            struct node *top = list_peek(stack);
            list_append(top->xml.children, t_node);
        }
    }

    root->type = ROOT_NODE;
    root->xml.name = strdup("ROOT");
    return root;
}

void print_parse_tree(const struct node *root, int depth) {
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

int process_node(struct container *c, const struct node *n) {
    // This function only expects XML nodes, not text or dummy nodes
    if (n->type != XML_NODE) return TRSS_ERR;
    if (!n->xml.children->count) return TRSS_ERR;

    struct node *text_node = (struct node *)(n->xml.children->elements[0]);
    if (text_node->type != TEXT_NODE) return TRSS_ERR; 

    const char *node_name = n->xml.name;
    if (c->type == ITEM) {
        struct item *item = c->item;
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
           item->pub_date = strdup(text_node->text);
        } else if (!strcmp(node_name, "description")) {
           item->description = strdup(text_node->text);
        } else {
            // printf("No place for %s in container type %i\n", node_name, c->type);
        }
    } else if (c->type == CHANNEL) {
        struct channel *channel = c->channel;
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

bool build_channel(struct channel *chan, struct node *root_node) {
    // Perform iterative DFS to build a channel from a parse tree

    struct list *container_stack = list_init();
    
    struct list *dfs_stack = list_init();
    list_append(dfs_stack, root_node); 

    while (!list_is_empty(dfs_stack)) {
        struct node *node = list_pop(dfs_stack);

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
                    struct container *new_item = container_init(ITEM);
                    struct container *parent = list_peek(container_stack);
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
                    struct container *root_container = malloc(sizeof(*root_container));
                    root_container->type = CHANNEL;
                    root_container->channel = chan;
                    list_append(container_stack, root_container);

                    for (ssize_t i = node->xml.children->count - 1; i >= 0; i--) {
                        struct node *n = node->xml.children->elements[i];
                        list_append(dfs_stack, n);
                    }
                } else if (list_is_empty(container_stack)) {
                    // Append node's children backwards so that the stack has the 
                    // leftmost child on the top
                    for (ssize_t i = node->xml.children->count - 1; i >= 0; i--) {
                        struct node *n = node->xml.children->elements[i];
                        list_append(dfs_stack, n);
                    }
                } else {
                    struct container *c = list_peek(container_stack);
                    if (process_node(c, node) == TRSS_ERR) {
                        fprintf(stderr, "Error processing node: %s\n", node->xml.name);
                    }
                }
                break;
            default:
                // fprintf(stderr, "Encountered unrecognized node!\n");
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

// ======== Initializers ======== //

struct item *item_init(void) {
    struct item *new_item = calloc(1, sizeof(*new_item));
    if (!new_item) {
        return NULL;
    }
    
    return new_item;
}

struct channel *channel_init(void) {
    struct channel *new_channel = calloc(1, sizeof(*new_channel));
    if (!new_channel) {
        return NULL;
    }

    new_channel->items = list_init();
    return new_channel;
}

struct container *container_init(enum CONTAINER_TYPE t) {
    struct container *new_container = malloc(sizeof(*new_container));
    
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

void free_item(struct item *it) {
    free(it->title);
    free(it->description);
    free(it->author);
    free(it->guid);
    free(it->link);
    free(it->pub_date);
    free(it);
}

void free_channel(struct channel *c) {
    free(c->description);
    free(c->title);
    free(c->language);
    free(c->last_build_date);
    free(c->link);

    for (size_t i = 0; i < c->items->count; i++) {
        struct item *it = c->items->elements[i];
        free_item(it);
    }
    free(c);
}

void free_container(struct container *c) {
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