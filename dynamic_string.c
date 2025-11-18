#include "dynamic_string.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct string_d *string_d_init(char * s) {
    struct string_d *new_s = malloc(sizeof(struct string_d));
    
    if (!new_s) {
        return NULL;
    }

    new_s->length = strlen(s);
    new_s->capacity = new_s->length * 2;
    new_s->str = strndup(s, new_s->length);

    return new_s;
}

struct string_d *string_d_clone(struct string_d *s) {
    if (!s) {
        return string_d_init("");
    }

    struct string_d *new_s = malloc(sizeof(struct string_d));

    if (!new_s) {
        fprintf(stderr, "Could not make copy of string object! Memory allocation error\n");
        return NULL;
    }

    new_s->capacity = s->capacity;
    new_s->length = s->length;
    new_s->str = strndup(s->str, s->length);
    return new_s;
}

void string_d_reset(struct string_d *s) {
    if (!s) return;
    free(s->str);
    s->str = malloc(1);
    if (s->str) {
        s->str[0] = '\0';
    }
    s->length = 0;
    s->capacity = 1;
}


bool string_d_append(struct string_d *s1, char *s2) {
    size_t length = strlen(s2);

    bool needs_extension = false;
    while (s1->length + length >= s1->capacity) {
        s1->capacity *= 2;
        needs_extension = true;
    }

    if (needs_extension) {
        char *tmp = realloc(s1->str, s1->capacity);
        if (!tmp) {
            fprintf(stderr, "Could not reallocate space for string!\n");
            return false;
        }
        s1->str = tmp;
    }
    snprintf(s1->str+length, s1->capacity, "%s", s2);
    s1->length += length;
    return true;
}

void string_d_free(struct string_d *s) {
    free(s->str);
    free(s);
    // s = NULL;
}