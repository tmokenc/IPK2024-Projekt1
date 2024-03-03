/**
 * @file bytes.c
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 02/03/2024
 *
 * Implementation of the `bytes.h` module
 */

#include "bytes.h"
#include "error.h"
#include <string.h>

size_t SIZE_CHUNK = 10;

Bytes bytes_new() {
    Bytes res;
    res.data = NULL;
    res.capacity = 0;
    res.len = 0;
    return res;
}

void bytes_free(Bytes *bytes) {
    if (bytes->capacity) {
        free(bytes->data);
        bytes->data = NULL;
        bytes->len = 0;
        bytes->capacity = 0;
    }
}

void bytes_realloc(Bytes *bytes, ssize_t new_capacity) {
    size_t capacity = new_capacity < 0 ? bytes->len : (size_t)new_capacity; 

    uint8_t *tmp = realloc(bytes->data, sizeof(uint8_t) * capacity);

    if (!tmp) {
        set_error(Error_OutOfMemory);
        return;
    }

    bytes->data = tmp;
    bytes->capacity = capacity;

    if (bytes->len > capacity) {
        bytes->len = capacity;
    }
}

void bytes_push(Bytes *bytes, uint8_t byte) {
    uint8_t arr[1];
    arr[0] = byte;
    bytes_push_arr(bytes, arr, 1);
}

void bytes_push_arr(Bytes *bytes, const uint8_t *arr, size_t len) {
    size_t new_len = bytes->len + len;
    if (new_len >= bytes->capacity) {
        bytes_realloc(bytes, bytes->capacity + SIZE_CHUNK);
        if (get_error()) {
            return;
        }
    }

    for (size_t i = 0; i < len; i++) {
        bytes->data[bytes->len + i] = arr[i];
    }

    bytes->len = new_len;
}


void bytes_push_c_str(Bytes *bytes, const char *str) {
    size_t size = strlen(str);
    bytes_push_arr(bytes, (const uint8_t *)str, size);
}

void bytes_remove_first_n(Bytes *bytes, size_t n) {
    if (bytes->len > n) {
        n = bytes->len;
    }

    for (size_t i = 0; i < (bytes->len - n); i++) {
        bytes[i] = bytes[i + n];
    }

    bytes->len -= n;
}
