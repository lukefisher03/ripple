#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define CHUNK_SIZE 4096
#define MAX_FILE_SIZE 5000000

char *file_to_string(const char *path, size_t *size);

bool sstartswith(const char *prefix, const char *str, size_t str_length);
bool sstartswith_fast(const char *prefix, size_t prefix_length, const char *str, size_t str_length);
bool rfc_822_to_tm(char *timestamp, struct tm *tm);

#endif