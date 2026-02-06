#include "list.h"

#include <string.h>

#define DEFAULT_CAPACITY 4

generic_list *list_init(void) {
    generic_list *l = malloc(sizeof(*l));
    if (!l) {
        return NULL;
    }
    l->capacity = DEFAULT_CAPACITY;
    l->count = 0;
    l->elements = calloc(DEFAULT_CAPACITY, sizeof(void *));

    return l;
}

bool list_append(generic_list *l, void *item) {
    // Push a new item onto the stack.
    if (l->count == l->capacity) {
        // Adding this element would fill the stack entirely
        l->capacity *= 2;
        void *tmp = realloc(l->elements, l->capacity * (sizeof(void *)));
        if (!tmp) {
            // Unable to reallocate the stack
            return false;
        }
        l->elements = tmp; // Reassign stack to tmp
    }
    l->elements[l->count++] = item;
    return true;
}

inline bool list_is_empty(const generic_list *l) { return l->count == 0; }

void *list_peek(const generic_list *l) {
    if (l->count == 0) {
        return NULL;
    }

    return l->elements[l->count - 1];
}

void *list_pop(generic_list *l) {
    // Same as peek except it decrements the stack counter, effectively popping
    // an element.
    if (l->count == 0) {
        return NULL;
    }
    void *e = l->elements[l->count - 1];
    l->count--;
    return e;
}

inline void list_clear(generic_list *l) { l->count = 0; }

void list_free(generic_list *l) {
    if (!l) return;
    free(l->elements);
    free(l);
}