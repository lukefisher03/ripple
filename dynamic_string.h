#ifndef DYNAMIC_STRING_H
#define DYNAMIC_STRING_H

#include <stdio.h>
#include <stdbool.h>

struct string_d {
    char    *str;
    size_t  capacity;
    size_t  length;
};

struct string_d *string_d_init(char * s);
struct string_d *string_d_clone(struct string_d *s);
bool string_d_append(struct string_d *s1, char *s2);
void string_d_reset(struct string_d *s);
void string_d_free(struct string_d *s);

#endif