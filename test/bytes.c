#include "greatest.h"

SUITE(bytes);

TEST bytes_new(void) {
    SKIPm("TODO");
}

TEST bytes_free(void) {
    SKIPm("TODO");
}

TEST bytes_realloc(void) {
    SKIPm("TODO");
}

TEST bytes_push(void) {
    SKIPm("TODO");
}

TEST bytes_push_arr(void) {
    SKIPm("TODO");
}

TEST bytes_push_c_str(void) {
    SKIPm("TODO");
}

TEST bytes_remove_first_n(void) {
    SKIPm("TODO");
}


GREATEST_SUITE(bytes) {
    RUN_TEST(bytes_new);
    RUN_TEST(bytes_free);
    RUN_TEST(bytes_realloc);
    RUN_TEST(bytes_push);
    RUN_TEST(bytes_push_arr);
    RUN_TEST(bytes_push_c_str);
    RUN_TEST(bytes_remove_first_n);
}

