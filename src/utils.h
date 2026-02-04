#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define CHUNK_SIZE 4096
#define MAX_FILE_SIZE 5000000

char *file_to_string(const char *path, size_t *size);
bool rfc_822_to_tm(char *timestamp, struct tm *tm);
int unix_time_to_formatted(int64_t unix_timestamp, char *str, size_t buf_len);

#endif