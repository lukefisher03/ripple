#ifndef ARENA_H
#define ARENA_H

#include <stdlib.h>
#include <stddef.h>

#define ARENA_DEFAULT_SIZE 4096 // Default arena size is 4KB
#define DEFAULT_ALIGNMENT _Alignof(max_align_t)

struct arena {
    size_t  capacity;
    size_t  fill_level;
    char    *block;
};

/* Advance the `fill_level` pointer in the block. Performs memory alignment */
void *arena_allocate(struct arena *arena, size_t size);

/* Initialize our allocator */
bool arena_init(struct arena *arena, size_t default_size);
/* Same as above except called with malloc instead of calloc */
bool arena_init_fast(struct arena *arena, size_t default_size);

/* Reset the fill_level pointer and zero the block so the arena can be reused */
void arena_reset(struct arena *arena);
/* Same as above except the block is not zeroed*/
void arena_reset_fast(struct arena *arena);

/* Free the block of memory, the arena itself is not freed */
void arena_free(struct arena *arena);

#endif