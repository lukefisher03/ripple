#include "list.h"

#include <string.h>


generic_list *list_init(void) {
    generic_list *l = malloc(sizeof(*l));
    if (!l) {
        return NULL;
    }
    l->capacity = LIST_DEFAULT_CAPACITY;
    l->count = 0;
    l->elements = calloc(LIST_DEFAULT_CAPACITY, sizeof(void *));

    if (!l->elements) {
        free(l);
        return NULL;
    }

    return l;
}

int list_append(generic_list *l, void *item) {
    // Push a new item onto the stack.
    if (l->count == l->capacity) {
        // Adding this element would fill the stack entirely
        l->capacity *= 2;
        void *tmp = realloc(l->elements, l->capacity * (sizeof(void *)));
        if (!tmp) {
            // Unable to reallocate the stack
            return 1;
        }
        l->elements = tmp; // Reassign stack to tmp
    }
    l->elements[l->count++] = item;
    return 0;
}

inline int list_empty(const generic_list *l) { return l->count == 0; }

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

bounded_list *bounded_list_init(size_t capacity) {
    if (capacity == 0) return NULL;

    bounded_list *l = malloc(sizeof(*l));

    if (!l) return NULL;
    l->elements = calloc(capacity, sizeof(void *));

    if (!l->elements) {
        free(l);
        return NULL;
    }

    l->capacity = capacity;
    l->front = -1;
    l->count = 0;
     
    return l;
}

int bounded_list_full(bounded_list *l) {
    return l->count == l->capacity;
}

int bounded_list_empty(bounded_list *l) {
    return l->count == 0;
}

void *bounded_list_append(bounded_list *l, void *element) {
    // Returns ejected element if the bounded list is full
    if (l->count < l->capacity) l->count++;

    // Advance 1, if that spot is full, then ejected will store the old pointer.
    l->front = (l->front + 1) % l->capacity;
    void *ejected = l->elements[l->front];

    l->elements[l->front] = element;  

    return ejected;
}

void *bounded_list_pop(bounded_list *l) {
    if (bounded_list_empty(l)) {
        return NULL;
    }

    void *element = l->elements[l->front];
    l->front = (l->front - 1 + l->capacity) % l->capacity;
    l->count--;
    return element;
}

void *bounded_list_peek(bounded_list *l) {
    if (bounded_list_empty(l)) return NULL;
    return l->elements[l->front];
}

void bounded_list_free(bounded_list *l) {
    free(l->elements);
    free(l);
}