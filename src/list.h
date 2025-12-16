#ifndef LIST_H
#define LIST_H

#include <stdbool.h>
#include <stdlib.h>

typedef struct List {
    size_t capacity;
    size_t count;
    void **elements;
} List;

List *list_init(void);

bool list_append(List *stk, void *item);

bool list_is_empty(const List *stk);

void *list_pop(List *stk);

void *list_peek(const List *stk);

void list_clear(List *stk);

void list_free(List *stk);

#endif