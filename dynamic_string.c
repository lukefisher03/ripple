#include "dynamic_string.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DEFAULT_BUFFER_SIZE 16

struct ds *string_d_init(const char * s) {
    struct ds *new_s = malloc(sizeof(*new_s));
    
    if (!new_s) {
        return NULL;
    }

    new_s->length = strlen(s);
    new_s->capacity = new_s->length * 2 > DEFAULT_BUFFER_SIZE ? new_s->length * 2 : DEFAULT_BUFFER_SIZE;
    new_s->str = malloc(new_s->capacity);
    
    if (!new_s->str) {
        free(new_s);
        return NULL;
    }

    memcpy(new_s->str, s, new_s->length + 1);
    return new_s;
}

struct ds *string_d_clone(const struct ds *s) {
    if (!s) {
        return string_d_init("");
    }

    struct ds *new_s = malloc(sizeof(*new_s));

    if (!new_s) {
        fprintf(stderr, "Could not make copy of string object! Memory allocation error\n");
        return NULL;
    }

    new_s->capacity = s->capacity;
    new_s->length = s->length;
    new_s->str = malloc(s->capacity);
    if (!new_s->str) {
        free(new_s);
        return NULL;
    }

    memcpy(new_s->str, s->str, s->length + 1);
    return new_s;
}

void string_d_reset(struct ds *s) {
    if (!s) return;
    free(s->str);
    s->str = malloc(DEFAULT_BUFFER_SIZE);
    if (s->str) {
        s->str[0] = '\0';
    }
    s->length = 0;
    s->capacity = DEFAULT_BUFFER_SIZE;
}


struct ds *string_d_append(struct ds *s1, const char *s2) {
    size_t length = strlen(s2);

    bool needs_extension = false;
    while (s1->length + length + 1 >= s1->capacity) {
        s1->capacity *= 2;
        needs_extension = true;
    }

    if (needs_extension) {
        char *tmp = realloc(s1->str, s1->capacity);
        if (!tmp) {
            fprintf(stderr, "Could not reallocate space for string!\n");
            return NULL;
        }
        s1->str = tmp;
    }
    memcpy(s1->str + s1->length, s2, length + 1);
    s1->length += length;

    return s1;
}

void string_d_free(struct ds *s) {
    free(s->str);
    free(s);
}