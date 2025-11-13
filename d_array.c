#include "d_array.h"

#include <stdio.h>
#include <string.h>

#define DEFAULT_CAPACITY 4

struct d_array *stack_init(void) {
    struct d_array *stk = malloc(sizeof(struct d_array));
    if (!stk) {
        return NULL;
    }
    stk->capacity = DEFAULT_CAPACITY;
    stk->count = 0;
    stk->elements = calloc(DEFAULT_CAPACITY, sizeof(void *));

    return stk;
}

bool stack_push(struct d_array *stk, void *item) {
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
    printf("Adding element at %lu\n", stk->count);
    stk->elements[stk->count++] = item;
    return true;
}

bool stack_empty(struct d_array *stk) {
    if (stk->count == 0) {
        return true;
    } 
    return false;
}

void *stack_pop(struct d_array *stk) {
    // Same as peek except it decrements the stack counter, effectively popping an element.
    if (stk->count == 0) {
        return NULL;
    }
    void *e = stk->elements[stk->count - 1];
    stk->count--;
    return e;
}

void stack_free(struct d_array *stk) {
    free(stk->elements);
    free(stk);
}