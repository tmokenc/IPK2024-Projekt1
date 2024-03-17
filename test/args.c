#include "greatest.h"
#include "../src/args.h"
#include "../src/error.h"

static void args_setup(void *arg) {
    set_error(Error_None);
    (void)arg;
}

SUITE(args);

TEST parse_nothing(void) {
    int argc = 1;
    char *argv[] = { "IPK2024" };

    parse_args(argc, argv);
    ASSERT_EQ(get_error(), Error_InvalidArgument);

    PASS();
}

TEST parse_default(void) {
    int argc = 5;
    char *argv[5] = { "test", "-t", "tcp", "-s", "test.com" };

    Args args = parse_args(argc, argv);

    ASSERT_FALSE(get_error());
    ASSERT_STR_EQ(args.host, "test.com");
    ASSERT_EQ(args.mode, Mode_TCP);
    ASSERT_EQ(args.port, 4567);
    ASSERT_EQ(args.udp_timeout, 250);
    ASSERT_EQ(args.udp_retransmissions, 3);

    argv[1] = "-s";
    argv[2] = "test.com";
    argv[3] = "-t";
    argv[4] = "udp";

    args = parse_args(argc, argv);

    ASSERT_FALSE(get_error());
    ASSERT_STR_EQ(args.host, "test.com");
    ASSERT_EQ(args.mode, Mode_UDP);
    ASSERT_EQ(args.port, 4567);
    ASSERT_EQ(args.udp_timeout, 250);
    ASSERT_EQ(args.udp_retransmissions, 3);

    PASS();
}

TEST parse_help(void) {
    int argc = 2;
    char *argv[2] = { "test", "-h" };

    Args args = parse_args(argc, argv);
    ASSERT_FALSE(get_error());
    ASSERT(args.help);

    PASS();
}

TEST parse_help_with_additional_args(void) {
    int argc = 4;
    char *argv[4] = { "test", "-h", "-t", "tcp" };

    parse_args(argc, argv);
    ASSERT(get_error());
    PASS();
}

TEST parse_missing_host(void) {
    int argc = 3;
    char *argv[3] = { "test", "-t", "tcp" };

    parse_args(argc, argv);
    ASSERT(get_error());

    PASS();
}

TEST parse_missing_mode(void) {
    int argc = 3;
    char *argv[3] = { "test", "-s", "test.com" };

    parse_args(argc, argv);
    ASSERT(get_error());

    PASS();
}

TEST parse_complete(void) {
    int argc = 11;
    char *argv[11] = { "test", "-t", "udp", "-s", "test.com", "-p", "1111", "-d", "888", "-r", "5" };

    Args args = parse_args(argc, argv);

    ASSERT_FALSE(get_error());
    ASSERT_STR_EQ(args.host, "test.com");
    ASSERT_EQ(args.mode, Mode_UDP);
    ASSERT_EQ(args.port, 1111);
    ASSERT_EQ(args.udp_timeout, 888);
    ASSERT_EQ(args.udp_retransmissions, 5);
    PASS();
}

TEST parse_repeat_argument(void) {
    int argc = 7;
    char *argv[7] = { "test", "-t", "udp", "-s", "test.com", "-t", "udp" };

    parse_args(argc, argv);
    ASSERT(get_error());
    PASS();
}

TEST parse_incorrect_order(void) {
    int argc = 5;
    char *argv[5] = { "test", "-t", "udp", "test.com", "-s" };

    parse_args(argc, argv);
    ASSERT(get_error());
    PASS();
}

GREATEST_SUITE(args) {
    GREATEST_SET_SETUP_CB(args_setup, NULL);
    RUN_TEST(parse_nothing);
    RUN_TEST(parse_default);
    RUN_TEST(parse_missing_host);
    RUN_TEST(parse_missing_mode);
    RUN_TEST(parse_help);
    RUN_TEST(parse_help_with_additional_args);
    RUN_TEST(parse_complete);
    RUN_TEST(parse_repeat_argument);
    RUN_TEST(parse_incorrect_order);
}
