#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stdlib.h>

#define CHUNK_SIZE 4096
#define MAX_FILE_SIZE 5000000

char *file_to_string(const char *path, size_t *size);

bool prefixcmp(const char *prefix, char *str, size_t str_length);
bool prefixcmp_fast(const char *prefix, size_t prefix_length, char *str, size_t str_length);

#endif