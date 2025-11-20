#ifndef D_ARRAY
#define D_ARRAY

#include <stdlib.h>
#include <stdbool.h>

struct dynamic_stack {
    size_t      capacity;
    size_t      count;
    void        **elements;
};

struct dynamic_stack *stack_init(void);

bool stack_push(struct dynamic_stack *stk, void *item);

bool stack_is_empty(struct dynamic_stack *stk);

void *stack_pop(struct dynamic_stack *stk);

void *stack_peek(struct dynamic_stack *stk);

void stack_clear(struct dynamic_stack *stk);

void stack_free(struct dynamic_stack *stk);

#endif