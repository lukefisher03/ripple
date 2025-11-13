#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stdlib.h>

#define CHUNK_SIZE 4096
#define MAX_FILE_SIZE 5000000

char *file_to_string(const char *path, size_t *size);

static inline bool alpha_test(unsigned char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

#endif