#ifndef D_ARRAY
#define D_ARRAY

#include <stdlib.h>
#include <stdbool.h>

struct d_array {
    size_t      capacity;
    size_t      count;
    void        **elements;
};

struct d_array *d_array_init(void);

bool d_array_append(struct d_array *arr, void *item);
bool d_array_empty(struct d_array *arr);
void *d_array_pop(struct d_array *arr);

void d_array_free(struct d_array *arr);
#endif