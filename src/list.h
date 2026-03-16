#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#define LIST_DEFAULT_CAPACITY 4

typedef struct {
    size_t capacity;
    size_t count;
    void **elements;
} generic_list;

generic_list *list_init(void);

int list_append(generic_list *stk, void *item);

int list_is_empty(const generic_list *stk);

void *list_pop(generic_list *stk);

void *list_peek(const generic_list *stk);

void list_clear(generic_list *stk);

void list_free(generic_list *stk);

#endif