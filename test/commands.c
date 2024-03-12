#include "greatest.h"
#include "../src/commands.h"
#include <string.h>
#include <stdint.h>
#include "../src/error.h"

static void commands_setup(void *arg) {
    command_setup();
    (void)arg;
}
static void commands_tear_down(void *arg) {
    command_clean_up();
    (void)arg;
}

SUITE(commands);

#define COMMAND_TEST(input, output_type, output_buf, output) do { \
    Command cmd = command_parse((void *)input); \
    ASSERT_FALSE(get_error()); \
    ASSERT_EQ(cmd.type, output_type); \
    ASSERT_STR_EQ(cmd.data.output_buf, output); \
} while (0)

static enum greatest_test_res auth_cmd_test(
        const void *input, 
        const void *username, 
        const void *secret,
        const void *display_name
) {
    Command cmd = command_parse(input);

    ASSERT_FALSE(get_error());
    ASSERT_EQ(cmd.type, CommandType_Auth);
    ASSERT_STR_EQ(cmd.data.auth.username, username);
    ASSERT_STR_EQ(cmd.data.auth.secret, secret);
    ASSERT_STR_EQ(cmd.data.auth.display_name, display_name);

    PASS();
}

static enum greatest_test_res invalid_command(const void *input) {
    command_parse(input);
    ASSERT(get_error());
    // clean up
    set_error(Error_None);
    PASS();
}

TEST no_command() {
    uint8_t data1[] = "Hello";
    uint8_t data2[] = "   Hi there";
    uint8_t data3[] = "Hmmmm    ";
    uint8_t data4[] = "/authh not commands";
    uint8_t data5[] = "//join not commands";

    COMMAND_TEST(data1, CommandType_None, message, data1);
    COMMAND_TEST(data2, CommandType_None, message, "Hi there");
    COMMAND_TEST(data3, CommandType_None, message, "Hmmmm");
    COMMAND_TEST(data4, CommandType_None, message, data4);
    COMMAND_TEST(data5, CommandType_None, message, data5);

    PASS();
}

TEST parse_auth() {
    uint8_t data1[] = "/auth hello tmokenc tomoka";
    uint8_t data2[] = "/auth user-name sec-Ret display_name";

    CHECK_CALL(auth_cmd_test(data1, "hello", "tmokenc", "tomoka"));
    CHECK_CALL(auth_cmd_test(data2, "user-name", "sec-Ret", "display_name"));

    CHECK_CALL(invalid_command("/auth username secret"));
    CHECK_CALL(invalid_command("/auth username display_name secret"));
    CHECK_CALL(invalid_command("/auth display_name username secret "));
    CHECK_CALL(invalid_command("/auth"));
    CHECK_CALL(invalid_command("/auth "));

    PASS();
}

TEST parse_join() {
    COMMAND_TEST("/join Testing", CommandType_Join, join.channel_id, "Tesing");
    COMMAND_TEST("/join Test-ing", CommandType_Join, join.channel_id, "Test-ing");

    CHECK_CALL(invalid_command("/join test ing"));
    CHECK_CALL(invalid_command("/join \0test ing"));
    CHECK_CALL(invalid_command("/join"));
    CHECK_CALL(invalid_command("/join "));

    PASS();
}

TEST parse_rename() {
    COMMAND_TEST("/rename tmokenc", CommandType_Rename, rename.display_name, "tmokenc");
    COMMAND_TEST("/rename tmok-enc", CommandType_Rename, rename.display_name, "tmok-enc");

    CHECK_CALL(invalid_command("/rename tmok enc"));
    CHECK_CALL(invalid_command("/rename"));
    CHECK_CALL(invalid_command("/rename "));

    PASS();
}

TEST _parse_help() {
    ASSERT_EQ(command_parse((void *)"/help").type, CommandType_Help);
    ASSERT_EQ(command_parse((void *)"/help   ").type, CommandType_Help);
    ASSERT_EQ(command_parse((void *)"   /help   ").type, CommandType_Help);

    PASS();
}

TEST parse_clear() {
    ASSERT_EQ(command_parse((void *)"/clear").type, CommandType_Clear);
    ASSERT_EQ(command_parse((void *)"/clear   ").type, CommandType_Clear);
    ASSERT_EQ(command_parse((void *)"   /clear   ").type, CommandType_Clear);

    PASS();
}

GREATEST_SUITE(commands) {
    GREATEST_SET_SETUP_CB(commands_setup, NULL);
    GREATEST_SET_TEARDOWN_CB(commands_tear_down, NULL);

    RUN_TEST(no_command);
    RUN_TEST(parse_auth);
    RUN_TEST(parse_join);
    RUN_TEST(parse_rename);
    RUN_TEST(_parse_help);
    RUN_TEST(parse_clear);
}
