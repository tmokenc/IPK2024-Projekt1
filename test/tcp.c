#include "greatest.h"
#include "../src/payload.h"
#include "../src/error.h"
#include "../src/bytes.h"
#include "../src/tcp.h"
#include <string.h>

Payload TCP_PAYLOAD;
Bytes TCP_BUFFER;

static void _tcp_setup(void *arg) {
    TCP_BUFFER = bytes_new();
    set_error(Error_None);
    tcp_setup();
    (void)arg;
}

static void tcp_tear_down(void *arg) {
    bytes_clear(&TCP_BUFFER);
    memset(&TCP_PAYLOAD, 0, sizeof(Payload));
    tcp_destroy();
    (void)arg;
}

SUITE(tcp);

TEST tcp_serialize_reply_ok(void) {
    char expect[] = "REPLY OK IS Nijigasaki Liella\r\n";


    TCP_PAYLOAD.type = PayloadType_Reply;
    TCP_PAYLOAD.data.reply.result = true;
    strcpy((void *)TCP_PAYLOAD.data.reply.message_content, "Nijigasaki Liella");

    tcp_serialize(&TCP_PAYLOAD, &TCP_BUFFER);

    ASSERT_FALSE(get_error());
    ASSERT_MEM_EQ(expect, TCP_BUFFER.data, TCP_BUFFER.len);
    PASS();
}

TEST tcp_serialize_reply_nok(void) {
    char expect[] = "REPLY NOK IS Nijigasaki Liella\r\n";

    TCP_PAYLOAD.type = PayloadType_Reply;
    TCP_PAYLOAD.data.reply.result = false;
    strcpy((void *)TCP_PAYLOAD.data.reply.message_content, "Nijigasaki Liella");

    tcp_serialize(&TCP_PAYLOAD, &TCP_BUFFER);

    ASSERT_FALSE(get_error());
    ASSERT_MEM_EQ(expect, TCP_BUFFER.data, TCP_BUFFER.len);
    PASS();
}


TEST tcp_serialize_auth(void) {
    char expect[] = "AUTH tomoka AS tmokenc USING MyUltimateSecret\r\n";

    TCP_PAYLOAD.type = PayloadType_Auth;
    strcpy((void *)TCP_PAYLOAD.data.auth.username, "tomoka");
    strcpy((void *)TCP_PAYLOAD.data.auth.display_name, "tmokenc");
    strcpy((void *)TCP_PAYLOAD.data.auth.secret, "MyUltimateSecret");

    tcp_serialize(&TCP_PAYLOAD, &TCP_BUFFER);

    ASSERT_FALSE(get_error());
    ASSERT_MEM_EQ(expect, TCP_BUFFER.data, TCP_BUFFER.len);
    PASS();
}

TEST tcp_serialize_join(void) {
    char expect[] = "JOIN Vietnamese AS tmokenc\r\n";

    TCP_PAYLOAD.type = PayloadType_Join;
    strcpy((void *)TCP_PAYLOAD.data.join.channel_id, "Vietnamese");
    strcpy((void *)TCP_PAYLOAD.data.join.display_name, "tmokenc");

    tcp_serialize(&TCP_PAYLOAD, &TCP_BUFFER);

    ASSERT_FALSE(get_error());
    ASSERT_MEM_EQ(expect, TCP_BUFFER.data, TCP_BUFFER.len);

    PASS();
}

TEST tcp_serialize_msg(void) {
    char expect[] = "MSG FROM tmokenc IS Nijigasaki Liella\r\n";

    TCP_PAYLOAD.type = PayloadType_Message;
    strcpy((void *)TCP_PAYLOAD.data.message.display_name, "tmokenc");
    strcpy((void *)TCP_PAYLOAD.data.message.message_content, "Nijigasaki Liella");

    tcp_serialize(&TCP_PAYLOAD, &TCP_BUFFER);

    ASSERT_FALSE(get_error());
    ASSERT_MEM_EQ(expect, TCP_BUFFER.data, TCP_BUFFER.len);

    PASS();
}

TEST tcp_serialize_err(void) {
    char expect[] = "ERR FROM tmokenc IS Nijigasaki Liella\r\n";

    TCP_PAYLOAD.type = PayloadType_Err;
    strcpy((void *)TCP_PAYLOAD.data.err.display_name, "tmokenc");
    strcpy((void *)TCP_PAYLOAD.data.err.message_content, "Nijigasaki Liella");

    tcp_serialize(&TCP_PAYLOAD, &TCP_BUFFER);

    ASSERT_FALSE(get_error());
    ASSERT_MEM_EQ(expect, TCP_BUFFER.data, TCP_BUFFER.len);

    PASS();
}


TEST tcp_serialize_bye(void) {
    char expect[] = "BYE\r\n";
    TCP_PAYLOAD.type = PayloadType_Bye;

    tcp_serialize(&TCP_PAYLOAD, &TCP_BUFFER);

    ASSERT_FALSE(get_error());
    ASSERT_EQ(TCP_BUFFER.len, 5);
    ASSERT_MEM_EQ(TCP_BUFFER.data, expect, 5);

    PASS();
}

TEST tcp_deserialize_reply_ok(void) {
    char data[] = "REPLY OK IS Nijigasaki Liella\r\n";

    TCP_PAYLOAD = tcp_deserialize((void *)data, strlen(data));

    ASSERT_FALSE(get_error());
    ASSERT_EQ(TCP_PAYLOAD.type, PayloadType_Reply);
    ASSERT(TCP_PAYLOAD.data.reply.result);
    ASSERT_STR_EQ(TCP_PAYLOAD.data.reply.message_content, "Nijigasaki Liella");
    PASS();
}

TEST tcp_deserialize_reply_nok(void) {
    char data[] = "REPLY NOK IS Nijigasaki Liella\r\n";

    TCP_PAYLOAD = tcp_deserialize((void *)data, strlen(data));

    ASSERT_FALSE(get_error());
    ASSERT_EQ(TCP_PAYLOAD.type, PayloadType_Reply);
    ASSERT_FALSE(TCP_PAYLOAD.data.reply.result);
    ASSERT_STR_EQ(TCP_PAYLOAD.data.reply.message_content, "Nijigasaki Liella");
    PASS();
}

TEST tcp_deserialize_auth(void) {
    char data[] = "AUTH tomoka AS tmokenc USING MyUltimateSecret\r\n";

    TCP_PAYLOAD = tcp_deserialize((void *)data, strlen(data));

    ASSERT_FALSE(get_error());
    ASSERT_EQ(TCP_PAYLOAD.type, PayloadType_Auth);
    ASSERT_STR_EQ(TCP_PAYLOAD.data.auth.display_name, "tmokenc");
    ASSERT_STR_EQ(TCP_PAYLOAD.data.auth.username, "tomoka");
    ASSERT_STR_EQ(TCP_PAYLOAD.data.auth.secret, "MyUltimateSecret");

    PASS();
}

TEST tcp_deserialize_join(void) {
    char data[] = "JOIN Vietnamese AS tmokenc\r\n";

    TCP_PAYLOAD = tcp_deserialize((void *)data, strlen(data));

    ASSERT_FALSE(get_error());
    ASSERT_EQ(TCP_PAYLOAD.type, PayloadType_Join);
    ASSERT_STR_EQ(TCP_PAYLOAD.data.join.display_name, "tmokenc");
    ASSERT_STR_EQ(TCP_PAYLOAD.data.join.channel_id, "Vietnamese");

    PASS();
}


TEST tcp_deserialize_msg(void) {
    char data[] = "MSG FROM tmokenc IS Nijigasaki Liella\r\n";

    TCP_PAYLOAD = tcp_deserialize((void *)data, strlen(data));

    ASSERT_FALSE(get_error());
    ASSERT_EQ(TCP_PAYLOAD.type, PayloadType_Message);
    ASSERT_STR_EQ(TCP_PAYLOAD.data.message.display_name, "tmokenc");
    ASSERT_STR_EQ(TCP_PAYLOAD.data.message.message_content, "Nijigasaki Liella");

    PASS();
}

TEST tcp_deserialize_err(void) {
    char data[] = "ERR FROM tmokenc IS Nijigasaki Liella\r\n";

    TCP_PAYLOAD = tcp_deserialize((void *)data, strlen(data));

    ASSERT_FALSE(get_error());
    ASSERT_EQ(TCP_PAYLOAD.type, PayloadType_Err);
    ASSERT_STR_EQ(TCP_PAYLOAD.data.err.display_name, "tmokenc");
    ASSERT_STR_EQ(TCP_PAYLOAD.data.err.message_content, "Nijigasaki Liella");

    PASS();
}

TEST tcp_deserialize_bye(void) {
    char data[] = "BYE\r\n";

    TCP_PAYLOAD = tcp_deserialize((void *)data, 5);

    ASSERT_FALSE(get_error());
    ASSERT_EQ(TCP_PAYLOAD.type, PayloadType_Bye);

    PASS();
}

static enum greatest_test_res tcp_deserialize_invalid_payload(char *msg, void *data) {
    tcp_deserialize(data, strlen(data));
    ASSERTm(msg, get_error());
    // clean up
    set_error(Error_None);

    PASS();
}

void tcp_gen_oversized_buf(char *start, int size, char *end) {
    bytes_push_c_str(&TCP_BUFFER, start);

    for (int i = 0; i < size + 1; i++) {
        bytes_push(&TCP_BUFFER, 'a');
    }

    bytes_push_c_str(&TCP_BUFFER, end);
    bytes_push(&TCP_BUFFER, 0);
}

TEST tcp_deserialize_invalid_username(void) {
    // empty
    CHECK_CALL(tcp_deserialize_invalid_payload("empty", "AUTH AS tmokenc USING MyUltimateSecret\r\n"));
    // contains_invalid_character
    CHECK_CALL(tcp_deserialize_invalid_payload("contains invalid character", "AUTH tom\x05oka AS tmokenc USING MyUltimateSecret\r\n"));
    // oversized
    tcp_gen_oversized_buf("AUTH ", USERNAME_LEN, " AS tmokenc USING MyUltimateSecret\r\n");
    CHECK_CALL(tcp_deserialize_invalid_payload("oversized", TCP_BUFFER.data));
    PASS();
}

TEST tcp_deserialize_invalid_channel_id(void) {
    // empty
    CHECK_CALL(tcp_deserialize_invalid_payload("empty", "JOIN  AS tmokenc\r\n"));
    // contains_invalid_character
    CHECK_CALL(tcp_deserialize_invalid_payload("contains invalid character", "JOIN Vietnamese.Czech AS tmokenc\r\n"));
    // oversized
    tcp_gen_oversized_buf("JOIN ", CHANNEL_ID_LEN, " AS tmokenc\r\n");
    CHECK_CALL(tcp_deserialize_invalid_payload("oversized", TCP_BUFFER.data));
    PASS();
}

TEST tcp_deserialize_invalid_secret(void) {
    // empty
    CHECK_CALL(tcp_deserialize_invalid_payload("empty", "AUTH tomoka AS tmokenc USING \r\n"));
    // contains_invalid_character
    CHECK_CALL(tcp_deserialize_invalid_payload("contains invalid character", "AUTH tomoka AS tmokenc USING Something.un-usual\r\n"));

    // oversized
    tcp_gen_oversized_buf("AUTH tomoka AS tmokenc USING ", SECRET_LEN, "\r\n");
    CHECK_CALL(tcp_deserialize_invalid_payload("oversized", TCP_BUFFER.data));

    PASS();
}

TEST tcp_deserialize_invalid_display_name(void) {
    // empty
    CHECK_CALL(tcp_deserialize_invalid_payload("empty", "MSG FROM  IS Nijigasaki Liella\r\n"));
    // contains_invalid_character
    CHECK_CALL(tcp_deserialize_invalid_payload("contains invalid character", "MSG FROM invalid\x05 IS Nijigasaki Liella\r\n"));
    // oversized
    tcp_gen_oversized_buf("MSG FROM ", DISPLAY_NAME_LEN, " IS Nijigasaki Liella\r\n");
    CHECK_CALL(tcp_deserialize_invalid_payload("oversized", TCP_BUFFER.data));

    PASS();
}

TEST tcp_deserialize_invalid_message_content(void) {
    // empty
    CHECK_CALL(tcp_deserialize_invalid_payload("empty", "ERR FROM tmokenc IS \r\n"));
    // contains_invalid_character
    CHECK_CALL(tcp_deserialize_invalid_payload("contains invalid character", "ERR FROM invalid\x05 IS Nijigasaki Liella\r\n"));
    // oversized
    tcp_gen_oversized_buf("MSG FROM tmokenc IS ", MESSAGE_CONTENT_LEN, "\r\n");
    CHECK_CALL(tcp_deserialize_invalid_payload("oversized", TCP_BUFFER.data));
    PASS();
}

GREATEST_SUITE(tcp) {
    GREATEST_SET_SETUP_CB(_tcp_setup, NULL);
    GREATEST_SET_TEARDOWN_CB(tcp_tear_down, NULL);

    RUN_TEST(tcp_serialize_reply_ok);
    RUN_TEST(tcp_serialize_reply_nok);
    RUN_TEST(tcp_serialize_auth);
    RUN_TEST(tcp_serialize_join);
    RUN_TEST(tcp_serialize_msg);
    RUN_TEST(tcp_serialize_err);
    RUN_TEST(tcp_serialize_bye);

    RUN_TEST(tcp_deserialize_reply_ok);
    RUN_TEST(tcp_deserialize_reply_nok);
    RUN_TEST(tcp_deserialize_auth);
    RUN_TEST(tcp_deserialize_join);
    RUN_TEST(tcp_deserialize_msg);
    RUN_TEST(tcp_deserialize_err);
    RUN_TEST(tcp_deserialize_bye);

    RUN_TEST(tcp_deserialize_invalid_secret);
    RUN_TEST(tcp_deserialize_invalid_username);
    RUN_TEST(tcp_deserialize_invalid_channel_id);
    RUN_TEST(tcp_deserialize_invalid_display_name);
    RUN_TEST(tcp_deserialize_invalid_message_content);
}
