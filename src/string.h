#ifndef STRING_H
#define STRING_H

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    char *data;
    size_t capacity;
    size_t len;
} String;

extern size_t SIZE_CHUNK;

String string_new();

void string_free(String *str);

void string_push_char(String *str, char ch);

void string_append_c_str(String *str, char *c_str);

#endif
