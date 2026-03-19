#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#define LIST_DEFAULT_CAPACITY 4

typedef struct {
    size_t      capacity;
    size_t      count;
    void        **elements;
} generic_list;

typedef struct {
    size_t      capacity;
    size_t      count;
    void        **elements;
    size_t      front;
    size_t      back;
} bounded_list;

generic_list *list_init(void);
int list_append(generic_list *stk, void *item);
int list_empty(const generic_list *stk);
void *list_pop(generic_list *stk);
void *list_peek(const generic_list *stk);
void list_clear(generic_list *stk);
void list_free(generic_list *stk);


bounded_list *bounded_list_init(size_t capacity);
int bounded_list_full(bounded_list *l);
int bounded_list_empty(bounded_list *l);
void *bounded_list_append(bounded_list *l, void *element);
void *bounded_list_pop(bounded_list *l);
void *bounded_list_peek(bounded_list *l);
void bounded_list_free(bounded_list *l);
#endif