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

Bytes bytes_new() {
    Bytes res = {0};
    return res;
}

void bytes_clear(Bytes *bytes) {
    memset(bytes, 0, sizeof(Bytes));
}

const uint8_t *bytes_get(const Bytes *bytes) {
    return bytes->data + bytes->offset;
}

void bytes_push(Bytes *bytes, uint8_t byte) {
    uint8_t arr[1];
    arr[0] = byte;
    bytes_push_arr(bytes, arr, 1);
}

void bytes_push_arr(Bytes *bytes, const uint8_t *arr, size_t len) {
    size_t new_len = bytes->len + len;

    if (new_len > BYTES_SIZE) {
        set_error(Error_StackOverflow);
    }

    for (size_t i = 0; i < len; i++) {
        bytes->data[bytes->offset + bytes->len + i] = arr[i];
    }

    bytes->len = new_len;
}

void bytes_push_c_str(Bytes *bytes, const char *str) {
    size_t size = strlen(str);
    bytes_push_arr(bytes, (const uint8_t *)str, size);
}

size_t bytes_trim(Bytes *bytes, uint8_t ch) {
    size_t count = 0;

    while (bytes->data[bytes->len + bytes->offset - 1] == ch) {
        bytes->len -= 1;
        count += 1;
    }

    bytes->data[bytes->offset + bytes->len] = 0;

    while (bytes->data[bytes->offset] == ch) {
        bytes->offset += 1;
        count += 1;
        bytes->len -= 1;
    }

    return count;
}

void bytes_skip_first_n(Bytes *bytes, size_t n) {
    if (bytes->len < n) {
        n = bytes->len;
    }

    bytes->offset += n;
    bytes->len -= n;
}
