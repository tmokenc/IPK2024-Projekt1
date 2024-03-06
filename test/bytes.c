#include "greatest.h"
#include "../src/bytes.h"
#include "../src/error.h"

Bytes BYTES;

static void bytes_setup(void *arg) {
    BYTES = bytes_new();
    set_error(Error_None);
    (void)arg;
}

static void bytes_tear_down(void *arg) {
    bytes_free(&BYTES);
    (void)arg;
}

SUITE(bytes);

TEST _bytes_new(void) {
    BYTES = bytes_new();
    ASSERT_FALSE(get_error());
    ASSERT_EQ(BYTES.len, 0);
    ASSERT_EQ(BYTES.capacity, 0);
    PASS();
}

TEST _bytes_free(void) {
    bytes_push(&BYTES, '1');
    ASSERT_FALSE(get_error());
    ASSERT_EQ(BYTES.len, 1);

    bytes_free(&BYTES);
    ASSERT_FALSE(get_error());
    ASSERT_EQ(BYTES.len, 0);
    ASSERT_EQ(BYTES.capacity, 0);

    PASS();
}

TEST _bytes_realloc(void) {
    uint8_t arr[200] = {0};

    bytes_push_arr(&BYTES, arr, 200);
    ASSERT_EQ(BYTES.len, 200);

    bytes_realloc(&BYTES, 400);
    ASSERT_EQ(BYTES.len, 200);
    ASSERT_EQ(BYTES.capacity, 400);

    bytes_realloc(&BYTES, -1);
    ASSERT_EQ(BYTES.len, 200);
    ASSERT_EQ(BYTES.capacity, 200);

    bytes_realloc(&BYTES, 100);
    ASSERT_EQ(BYTES.len, 100);
    ASSERT_EQ(BYTES.capacity, 100);

    PASS();
}

TEST _bytes_push(void) {
    ASSERT_EQ(BYTES.len, 0);

    bytes_push(&BYTES, 1);
    ASSERT_FALSE(get_error());
    ASSERT_EQ(BYTES.len, 1);

    bytes_push(&BYTES, 2);
    ASSERT_FALSE(get_error());
    ASSERT_EQ(BYTES.len, 2);

    bytes_push(&BYTES, 3);
    ASSERT_FALSE(get_error());
    ASSERT_EQ(BYTES.len, 3);

    uint8_t arr[3] = {1, 2, 3};
    ASSERT_MEM_EQ(BYTES.data, arr, 3);

    PASS();
}

TEST _bytes_push_arr(void) {
    uint8_t arr[] = {1, 2, 3, 0};

    bytes_push_arr(&BYTES, arr, 4);
    ASSERT_EQ(BYTES.len, 4);
    ASSERT_MEM_EQ(BYTES.data, arr, 4);

    bytes_push_arr(&BYTES, arr, 3);
    uint8_t arr2[] = {1 ,2 ,3, 0, 1, 2, 3};
    ASSERT_EQ(BYTES.len, 7);
    ASSERT_MEM_EQ(BYTES.data, arr2, 7);

    bytes_push_arr(&BYTES, arr, 0);
    ASSERT_EQ(BYTES.len, 7);
    ASSERT_MEM_EQ(BYTES.data, arr2, 7);

    PASS();
}

TEST _bytes_push_c_str(void) {
    char str[] = "Hello";
    char str2[] = " WORLD";
    char empty[] = { 0 };

    uint8_t arr[] = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'O', 'R', 'L', 'D' };

    bytes_push_c_str(&BYTES, str);
    ASSERT_EQ(BYTES.len, 5);
    ASSERT_MEM_EQ(BYTES.data, arr, 5);

    bytes_push_c_str(&BYTES, str2);
    ASSERT_EQ(BYTES.len, 11);
    ASSERT_MEM_EQ(BYTES.data, arr, 11);

    bytes_push_c_str(&BYTES, empty);
    ASSERT_EQ(BYTES.len, 11);
    ASSERT_MEM_EQ(BYTES.data, arr, 11);

    PASS();
}

TEST _bytes_remove_first_n(void) {
    char str[] = "Hello WORLD";
    uint8_t arr[] = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'O', 'R', 'L', 'D' };
    bytes_push_c_str(&BYTES, str);
    ASSERT_EQ(BYTES.len, 11);
    ASSERT_MEM_EQ(BYTES.data, arr, 11);

    size_t old_capacity = BYTES.capacity;

    bytes_remove_first_n(&BYTES, 6);
    ASSERT_EQ(BYTES.len, 5);
    ASSERT_EQ(BYTES.capacity, old_capacity);
    ASSERT_MEM_EQ(BYTES.data, arr + 6, 5);

    PASS();
}

GREATEST_SUITE(bytes) {
    GREATEST_SET_SETUP_CB(bytes_setup, NULL);
    GREATEST_SET_TEARDOWN_CB(bytes_tear_down, NULL);

    RUN_TEST(_bytes_new);
    RUN_TEST(_bytes_free);
    RUN_TEST(_bytes_realloc);
    RUN_TEST(_bytes_push);
    RUN_TEST(_bytes_push_arr);
    RUN_TEST(_bytes_push_c_str);
    RUN_TEST(_bytes_remove_first_n);
}

