#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "arena.h"

bool is_power_of_two(uintptr_t x) {
    return (x & (x-1)) == 0;
}

void *arena_allocate_align(struct arena *arena, size_t size, uintptr_t align) {
    assert(arena);
    assert(is_power_of_two(align));

    uintptr_t cur_level = (uintptr_t)arena->block + arena->fill_level;
    uintptr_t offset = (align - (cur_level & (align - 1))) & (align - 1);
    size_t required = arena->fill_level + offset + size;
    if (arena->capacity < required) {
        fprintf(stderr, "Allocator ran out of memory, please allocate more. %lu/%lu bytes\n", required, arena->capacity);
        abort(); // Aborting on overflow minimizes the chance of memory errors.
    }

    void *p = arena->block + arena->fill_level + offset;
    arena->fill_level = required;
    return p;
}

void *arena_allocate(struct arena *arena, size_t size) {
    return arena_allocate_align(arena, size, DEFAULT_ALIGNMENT);
}

bool _arena_init(struct arena *arena, size_t default_size, bool clear_block) {
    if (clear_block) {
        arena->block = calloc(default_size, 1); // Memory is zeroed initially
    } else {
        arena->block = malloc(default_size);
    }
    if (!arena->block) return false;

    arena->capacity = default_size;
    arena->fill_level = 0;
    return true;
}

bool arena_init(struct arena *arena, size_t default_size) {
    return _arena_init(arena, default_size, true);
}

bool arena_init_fast(struct arena *arena, size_t default_size) {
    return _arena_init(arena, default_size, false);
}

void arena_reset_fast(struct arena *arena) {
    arena->fill_level = 0;
}

void arena_reset(struct arena *arena) {
    arena_reset_fast(arena);
    memset(arena->block, 0, arena->capacity);
}

void arena_free(struct arena *arena) {
    free(arena->block);
    arena->block = NULL;
    arena->capacity = 0;
    arena->fill_level = 0;
}

int main() {
    struct arena a;
    arena_init(&a, 50);

    int *x = arena_allocate(&a, sizeof(*x));
    char *s = "Hello world, this is neat!";
    size_t l = strlen(s);
    char *y = arena_allocate(&a, l+1);
    strncpy(y, s, l+1);
    printf("%s\n", y);
    int *z = arena_allocate(&a, sizeof(*z));
    
}