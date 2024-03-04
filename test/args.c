#include "greatest.h"
#include "../src/args.h"
#include "../src/error.h"

SUITE(args);

TEST parse_nothing(void) {
    int argc = 1;
    char *argv[] = { "IPK2024" };

    parse_args(argc, argv);
    ASSERT_EQ(get_error(), Error_InvalidArgument);

    PASS();
}

TEST parse_default(void) {
    SKIPm("TODO");
}

TEST parse_missing_host(void) {
    SKIPm("TODO");
}

TEST parse_missing_port(void) {
    SKIPm("TODO");
}

TEST parse_complete(void) {
    SKIPm("TODO");
}

TEST parse_repeat_argument(void) {
    SKIPm("TODO");
}

TEST parse_incorrect_order(void) {
    SKIPm("TODO");
}

GREATEST_SUITE(args) {
    RUN_TEST(parse_nothing);
    RUN_TEST(parse_default);
    RUN_TEST(parse_missing_host);
    RUN_TEST(parse_missing_port);
    RUN_TEST(parse_complete);
    RUN_TEST(parse_repeat_argument);
    RUN_TEST(parse_incorrect_order);
}

