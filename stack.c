#include "stack.h"

#include <string.h>

#define DEFAULT_CAPACITY 4

struct dynamic_stack *stack_init(void) {
    struct dynamic_stack *stk = malloc(sizeof(struct dynamic_stack));
    if (!stk) {
        return NULL;
    }
    stk->capacity = DEFAULT_CAPACITY;
    stk->count = 0;
    stk->elements = calloc(DEFAULT_CAPACITY, sizeof(void *));

    return stk;
}

bool stack_push(struct dynamic_stack *stk, void *item) {
    // Push a new item onto the stack.
    if (stk->count == stk->capacity) {
        // Adding this element would fill the stack entirely
        stk->capacity *= 2;
        void *tmp = realloc(stk->elements, stk->capacity * (sizeof(void *)));
        if (!tmp) {
            // Unable to reallocate the stack
            return false;
        }
        stk->elements = tmp; // Reassign stack to tmp
    }
    stk->elements[stk->count++] = item;
    return true;
}

inline bool stack_is_empty(struct dynamic_stack *stk) {
    return stk->count == 0;
}

void *stack_peek(struct dynamic_stack *stk) {
    if (stk->count == 0) {
        return NULL;
    }

    return stk->elements[stk->count - 1];
}

void *stack_pop(struct dynamic_stack *stk) {
    // Same as peek except it decrements the stack counter, effectively popping an element.
    if (stk->count == 0) {
        return NULL;
    }
    void *e = stk->elements[stk->count - 1];
    stk->count--;
    return e;
}

inline void stack_clear(struct dynamic_stack *stk) {
    stk->count = 0;
}

void stack_free(struct dynamic_stack *stk) {
    free(stk->elements);
    free(stk);
}