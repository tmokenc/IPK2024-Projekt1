#include "string.h"
#include "error.h"

size_t SIZE_CHUNK = 10;

String string_new() {
    String res;
    res.data = NULL;
    res.capacity = 0;
    res.len = 0;
    return res;
}

void string_free(String *str) {
    if (str->capacity) {
        free(str->data);
        str->data = NULL;
        str->len = 0;
        str->capacity = 0;
    }
}

static void string_raise_capacity(String *str, size_t new_capacity) {
    char *tmp = realloc(str->data, sizeof(char) * new_capacity);

    if (!tmp) {
        set_error(Error_OutOfMemory);
        return;
    }

    str->data = tmp;
    str->capacity = new_capacity;
}

void string_push_char(String *str, char ch) {
    size_t new_len = str->len + 2;
    if (new_len >= str->capacity) {
        string_raise_capacity(str, str->capacity + SIZE_CHUNK);
        if (get_error()) {
            return;
        }
    }

    str->data[str->len++] = ch;
    str->data[str->len] = '\0';
}
