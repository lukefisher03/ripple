#include "utils.h"
#include <stdio.h>
#include <string.h>

char *file_to_string(const char *path, size_t *size) {
    char *str; // Buffer for storing the output file string content
    FILE *fptr;
    
    fptr = fopen(path, "rb");
    if (!fptr) {
        fprintf(stderr, "Could not open file! %s\n", path);
        return NULL;
    }

    size_t capacity = 100000; // 100KB base size
    size_t buf_sz = 0; 

    str = (char *)malloc(capacity);
    if (!str) {
        fprintf(stderr, "Could not allocate space to hold file %s\n", path);
        fclose(fptr);
        return NULL;
    }

    size_t bytes_read;
    while (1) {
        if (buf_sz + CHUNK_SIZE >= MAX_FILE_SIZE - 1) {
            // Exit if the file goes over MAX_FILE_SIZE. (Minus 1 for null terminator)
            fprintf(stderr, "File is too big. Must be smaller than %i bytes\n", MAX_FILE_SIZE);
            fclose(fptr);
            free(str);
            return NULL;
        }
        if (buf_sz + CHUNK_SIZE >= capacity - 1) {
            // Expand the array if the buffer reaches capacity.
            capacity *= 2;
            char *tmp = realloc(str, capacity);
            if (!tmp) {
                fprintf(stderr, "Memory allocation error!\n");
                fclose(fptr);
                free(str);
                return NULL;
            }
            str = tmp;
        }

        bytes_read = fread(str + buf_sz, 1, CHUNK_SIZE, fptr); // Read 1 byte at a time.
        buf_sz += bytes_read;
        if (bytes_read < CHUNK_SIZE) {
            // Exit if the number of bytes read is less than the chunk size. This
            // means we're done.
            if (!feof(fptr)) {
                // If bytes read is less than the chunk size but no EOF was logged,
                // then we know that an error occurred.
                perror("read error");
                free(str);
                fclose(fptr);
                return NULL;
            }
            break;
        }
    }

    str[buf_sz] = '\0'; // Add a null terminating character so it's useable.

    fclose(fptr);
    *size = buf_sz;
    return str;
}

bool prefixcmp(const char *prefix, char *str, size_t str_length) {
    /*  Prefix compare - Given some prefix and a string to compare it against, 
        check if the string starts with the prefix  */

    if (!strlen(prefix)) return true; // A zero length prefix always returns true.
    if (!str_length) return false; // If the prefix is non-zero and the string is 0, return false.
    
    for (size_t i = 0; i < strlen(prefix) && i < str_length; i++) {
        if (prefix[i] != str[i]) {
            return false;
        }
    }
    
    return true;
}

bool prefixcmp_fast(const char *prefix, size_t prefix_length, char *str, size_t str_length) {
    /* Prefix compare fast - Same as prefix compare, except with no calls to O(n) strlen. 
       Use for comparing larger strings where the cost of O(n) is too high. */

    if (!prefix_length) return true; // A zero length prefix always returns true.
    if (!str_length) return false; // If the prefix is non-zero and the string is 0, return false.
    
    for (size_t i = 0; i < prefix_length && i < str_length; i++) {
        if (prefix[i] != str[i]) {
            return false;
        }
    }
    
    return true;
}