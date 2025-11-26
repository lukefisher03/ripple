#include "xml_rss.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>

struct node *construct_parse_tree(char *xml, size_t length) {
    // Initialize root node
    struct node *root = xml_node_init();
    struct list *stack = list_init();
    list_append(stack, root);

    // counter variable
    size_t i = 0;
    while (i < length) {
        if (xml[i] == '\n' || xml[i] == '\t' || xml[i] == ' ') {
            i++;
            continue;
        }
        // Store current position pointer
        char *s = xml + i;
        size_t l = length - i;
        
        if (prefixcmp("<", s, l) && !prefixcmp("<!", s, l) && !prefixcmp("<?", s, l)) {
            struct tag new_tag;
            if (read_tag(s, l, &new_tag)) { // Read the tag
                if (new_tag.tag_type == TAG_OPEN) {
                    struct node *top = list_peek(stack); // Look at the top stack element

                    struct node *node = xml_node_init(); // 
                    node->xml.name = strndup(new_tag.name, strlen(new_tag.name));
                    list_append(top->xml.children, node);
                    list_append(stack, node);

                } else if (new_tag.tag_type == TAG_CLOSE) {
                    struct node *v = list_pop(stack); // Pop off the stack
                }

                i += new_tag.total_length+1;
            }
        } else {
            struct node *t_node = text_node_init();
            ssize_t text_length = accumulate_text(s, l, t_node);

            if (text_length < 1) {
                printf("Error processing text node!\n");
                i++;
                continue;
            } 

            i += text_length;
            struct node *top = list_peek(stack);
            list_append(top->xml.children, t_node);
        }
    }

    return root;
}

void print_parse_tree(struct node *root, int depth) {
    /* Recursive function for printing out an indented version of a
       parse tree */

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

char *get_spacer(int width) {
    char *spacer = calloc(width+1, sizeof(char));
    if (!spacer) {
        return NULL;
    }
    memset(spacer, '\t', width);
    spacer[width] = '\0';
    return spacer;
}

bool read_tag(char *str, size_t length, struct tag *t) {
    if (!t) return false;
    
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

    t->total_length = i;
    return true;
}

ssize_t accumulate_text(char *str, size_t length, struct node *new_node) {
    if (!new_node) {
        printf("EXITED DUE TO NOT HAVING NEW NODE!\n");
        return -1;
    }

    ssize_t i = 1;
    if (prefixcmp("<![CDATA[", str, length)) {
        for (; i < length && !prefixcmp("]]>", str + i, length - i); i++);
        i += 3;
    } else {
        for (; i < length && str[i] != '<'; i++); 
    }

    new_node->type = TEXT_NODE;
    new_node->text = strndup(str, i);
    return i;
}

bool build_channel(struct channel *chan, struct node *root) {
    /* Perform iterative DFS to build a channel from a parse tree */
    struct list *stack = list_init();
    list_append(stack, chan); // Initialize stack with the channel element

    struct node *pointer = root;
    struct list *dfs_stack = list_init(); // Stack for doing DFS
    list_append(dfs_stack, root); 

    while (!list_is_empty(dfs_stack)) {
        struct node *node = list_pop(dfs_stack);

        switch (node->type) {
            case XML_NODE:
                if (node->xml.name == "channel") {
                    
                }
                // Do processing
                for (size_t i = 0; i < node->xml.children->count; i++) {
                    // Append next nodes to visit on the stack
                    list_append(dfs_stack, node->xml.children->elements[i]);
                }
                break;
            case TEXT_NODE:

                break;
            default:
                fprintf(stderr, "Encountered unrecognized node!\n");
                break;
        }
    }

    // FREE STACKS
}


bool is_termination_char(char c) {
    if (c == ' ' || c == ':' || c == '/' || c == '>') {
        return true;
    }
    return false;
}

struct tag *tag_init() {
    struct tag *new_tag = malloc(sizeof(*new_tag));
    return new_tag ? new_tag : NULL;
}