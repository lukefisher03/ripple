#ifndef DYNAMIC_STRING_H
#define DYNAMIC_STRING_H

#include <stdio.h>
#include <stdbool.h>

struct ds { // The name `ds` stands for dynamic string
    char    *str;
    size_t  capacity;
    size_t  length;
};

struct ds *string_d_init(const char * s);
struct ds *string_d_clone(const struct ds *s);
struct ds *string_d_append(struct ds *s1, const char *s2);
void string_d_reset(struct ds *s);
void string_d_free(struct ds *s);

#endif