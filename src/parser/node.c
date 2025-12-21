#include "node.h"

#include <stdio.h>
#include <string.h>

// -------- Initializers -------- //
rss_node *xml_node_init(void) {
    rss_node *new_node = calloc(1, sizeof(*new_node));
    if (!new_node) {
        return NULL;
    }

    new_node->type = XML_NODE;
    new_node->xml.children = list_init();
    return new_node;
}

rss_node *text_node_init(void) {
    rss_node *new_node = calloc(1, sizeof(*new_node));
    if (!new_node) {
        return NULL;
    }

    new_node->type = TEXT_NODE;
    return new_node;
}

rss_node *dummy_node_init(void) {
    rss_node *new_node = calloc(1, sizeof(*new_node));
    if (!new_node) {
        return NULL;
    }

    new_node->type = DUMMY;
    return new_node;
}

// -------- Cleanup -------- //

static void free_text_node(rss_node *node) {
    free(node->text);
    free(node);
}

static void free_xml_node(rss_node *node) {
    free(node->xml.name);
    list_free(node->xml.children);
    free(node);
}

void free_node(rss_node *node) {
    switch (node->type) {
    case XML_NODE:
        free_xml_node(node);
        break;
    case TEXT_NODE:
        free_text_node(node);
        break;
    default:
        free(node);
        break;
    }
}

// TODO: Convert to iterative to avoid stack overflow
void free_tree(rss_node *node) {
    // Free an entire tree of nodes. The tree must ONLY contain
    // nodes defined in this file.
    if (!node) {
        return;
    }

    switch (node->type) {
        case DUMMY:
        case TEXT_NODE:
            free_node(node);
            break;
        case ROOT_NODE:
        case XML_NODE:
            for (size_t i = 0; i < node->xml.children->count; i++) {
                free_tree(node->xml.children->elements[i]);
            }
            free_node(node);
        default:
            break;
    }
}