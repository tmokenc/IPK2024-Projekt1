#include "greatest.h"
#include "../src/payload.h"
#include "../src/error.h"
#include "../src/bytes.h"
#include "../src/tcp.h"
#include <string.h>

Payload TCP_PAYLOAD;
Bytes TCP_BUFFER;

static void tcp_setup(void *arg) {
    TCP_BUFFER = bytes_new();
    set_error(Error_None);
    (void)arg;
}

static void tcp_tear_down(void *arg) {
    bytes_free(&TCP_BUFFER);
    (void)arg;
}

SUITE(tcp);

TEST serialize_tcp_reply_ok(void) {
    char expect[] = "REPLY OK IS Nijigasaki Liella\r\n";


    TCP_PAYLOAD.type = PayloadType_Reply;
    TCP_PAYLOAD.data.reply.result = true;
    memcpy(TCP_PAYLOAD.data.reply.message_content, "Nijigasaki Liella", strlen("Nijigasaki Liella"));

    tcp_serialize(&TCP_PAYLOAD, &TCP_BUFFER);

    ASSERT_FALSE(get_error());
    ASSERT_MEM_EQ(expect, TCP_BUFFER.data, TCP_BUFFER.len);
    PASS();
}

TEST serialize_tcp_reply_nok(void) {
    char expect[] = "REPLY NOK IS Nijigasaki Liella\r\n";


    TCP_PAYLOAD.type = PayloadType_Reply;
    TCP_PAYLOAD.data.reply.result = false;
    memcpy(TCP_PAYLOAD.data.reply.message_content, "Nijigasaki Liella", strlen("Nijigasaki Liella"));

    tcp_serialize(&TCP_PAYLOAD, &TCP_BUFFER);

    ASSERT_FALSE(get_error());
    ASSERT_MEM_EQ(expect, TCP_BUFFER.data, TCP_BUFFER.len);
    PASS();
}


TEST serialize_tcp_auth(void) {
    char expect[] = "AUTH tomoka AS tmokenc using MyUltimateSecret\r\n";

    TCP_PAYLOAD.type = PayloadType_Auth;
    memcpy(TCP_PAYLOAD.data.auth.username, "tomoka", strlen("tomoka"));
    memcpy(TCP_PAYLOAD.data.auth.display_name, "tmokenc", strlen("tmokenc"));
    memcpy(TCP_PAYLOAD.data.auth.secret, "MyUltimateSecret", strlen("MyUltimateSecret"));

    tcp_serialize(&TCP_PAYLOAD, &TCP_BUFFER);

    ASSERT_FALSE(get_error());
    ASSERT_MEM_EQ(expect, TCP_BUFFER.data, TCP_BUFFER.len);
    PASS();
}

TEST serialize_tcp_join(void) {
    char expect[] = "JOIN Vietnamese AS tmokenc\r\n";

    TCP_PAYLOAD.type = PayloadType_Join;
    memcpy(TCP_PAYLOAD.data.join.channel_id, "Vietnamese", strlen("Vietnamese"));
    memcpy(TCP_PAYLOAD.data.join.display_name, "tmokenc", strlen("tmokenc"));

    tcp_serialize(&TCP_PAYLOAD, &TCP_BUFFER);

    ASSERT_FALSE(get_error());
    ASSERT_MEM_EQ(expect, TCP_BUFFER.data, TCP_BUFFER.len);

    PASS();
}

TEST serialize_tcp_msg(void) {
    char expect[] = "MSG FROM tmokenc IS Nijigasaki Liella\r\n";

    TCP_PAYLOAD.type = PayloadType_Message;
    memcpy(TCP_PAYLOAD.data.message.display_name, "tmokenc", strlen("tmokenc"));
    memcpy(TCP_PAYLOAD.data.message.message_content, "Nijigasaki Liella", strlen("Nijigasaki Liella"));

    tcp_serialize(&TCP_PAYLOAD, &TCP_BUFFER);

    ASSERT_FALSE(get_error());
    ASSERT_MEM_EQ(expect, TCP_BUFFER.data, TCP_BUFFER.len);

    PASS();
}

TEST serialize_tcp_err(void) {
    char expect[] = "ERR FROM tmokenc IS Nijigasaki Liella\r\n";

    TCP_PAYLOAD.type = PayloadType_Err;
    memcpy(TCP_PAYLOAD.data.err.display_name, "tmokenc", strlen("tmokenc"));
    memcpy(TCP_PAYLOAD.data.err.message_content, "Nijigasaki Liella", strlen("Nijigasaki Liella"));

    tcp_serialize(&TCP_PAYLOAD, &TCP_BUFFER);

    ASSERT_FALSE(get_error());
    ASSERT_MEM_EQ(expect, TCP_BUFFER.data, TCP_BUFFER.len);

    PASS();
}


TEST serialize_tcp_bye(void) {
    char expect[] = "BYE\r\n";
    TCP_PAYLOAD.type = PayloadType_Bye;

    tcp_serialize(&TCP_PAYLOAD, &TCP_BUFFER);

    ASSERT_FALSE(get_error());
    ASSERT_EQ(TCP_BUFFER.len, 5);
    ASSERT_MEM_EQ(TCP_BUFFER.data, expect, 5);

    PASS();
}

TEST deserialize_tcp_reply_ok(void) {
    char data[] = "REPLY OK IS Nijigasaki Liella\r\n";

    TCP_PAYLOAD = tcp_deserialize((void *)data, strlen(data));

    ASSERT_FALSE(get_error());
    ASSERT_EQ(TCP_PAYLOAD.type, PayloadType_Reply);
    ASSERT_TRUE(TCP_PAYLOAD.data.reply.result);
    ASSERT_STR_EQ(TCP_PAYLOAD.data.reply.message_content, "Nijigasaki Liella");
    PASS();
}

TEST deserialize_tcp_reply_nok(void) {
    char data[] = "REPLY NOK IS Nijigasaki Liella\r\n";

    TCP_PAYLOAD = tcp_deserialize((void *)data, strlen(data));

    ASSERT_FALSE(get_error());
    ASSERT_EQ(TCP_PAYLOAD.type, PayloadType_Reply);
    ASSERT_TRUE(TCP_PAYLOAD.data.reply.result);
    ASSERT_STR_EQ(TCP_PAYLOAD.data.reply.message_content, "Nijigasaki Liella");
    PASS();
}

TEST deserialize_tcp_auth(void) {
    char data[] = "AUTH tomoka AS tmokenc using MyUltimateSecret\r\n";

    TCP_PAYLOAD = tcp_deserialize((void *)data, strlen(data));

    ASSERT_FALSE(get_error());
    ASSERT_EQ(TCP_PAYLOAD.type, PayloadType_Auth);
    ASSERT_STR_EQ(TCP_PAYLOAD.data.auth.display_name, "tmokenc");
    ASSERT_STR_EQ(TCP_PAYLOAD.data.auth.username, "tomoka");
    ASSERT_STR_EQ(TCP_PAYLOAD.data.auth.secret, "MyUltimateSecret");

    PASS();
}

TEST deserialize_tcp_join(void) {
    char data[] = "JOIN Vietnamese AS tmokenc\r\n";

    TCP_PAYLOAD = tcp_deserialize((void *)data, strlen(data));

    ASSERT_FALSE(get_error());
    ASSERT_EQ(TCP_PAYLOAD.type, PayloadType_Join);
    ASSERT_STR_EQ(TCP_PAYLOAD.data.join.display_name, "tmokenc");
    ASSERT_STR_EQ(TCP_PAYLOAD.data.join.channel_id, "Vietnamese");

    PASS();
}


TEST deserialize_tcp_msg(void) {
    char data[] = "MSG FROM tmokenc IS Nijigasaki Liella\r\n";

    TCP_PAYLOAD = tcp_deserialize((void *)data, strlen(data));

    ASSERT_FALSE(get_error());
    ASSERT_EQ(TCP_PAYLOAD.type, PayloadType_Message);
    ASSERT_STR_EQ(TCP_PAYLOAD.data.err.display_name, "tmokenc");
    ASSERT_STR_EQ(TCP_PAYLOAD.data.err.message_content, "Nijigasaki Liella");

    PASS();
}

TEST deserialize_tcp_err(void) {
    char data[] = "ERR FROM tmokenc IS Nijigasaki Liella\r\n";

    TCP_PAYLOAD = tcp_deserialize((void *)data, strlen(data));

    ASSERT_FALSE(get_error());
    ASSERT_EQ(TCP_PAYLOAD.type, PayloadType_Err);
    ASSERT_STR_EQ(TCP_PAYLOAD.data.err.display_name, "tmokenc");
    ASSERT_STR_EQ(TCP_PAYLOAD.data.err.message_content, "Nijigasaki Liella");

    PASS();
}

TEST deserialize_tcp_bye(void) {
    char data[] = "BYE\r\n";

    TCP_PAYLOAD = tcp_deserialize((void *)data, 5);

    ASSERT_FALSE(get_error());
    ASSERT_EQ(TCP_PAYLOAD.type, PayloadType_Bye);

    PASS();
}

GREATEST_SUITE(tcp) {
    GREATEST_SET_SETUP_CB(tcp_setup, NULL);
    GREATEST_SET_TEARDOWN_CB(tcp_tear_down, NULL);

    RUN_TEST(serialize_tcp_reply_ok);
    RUN_TEST(serialize_tcp_reply_nok);
    RUN_TEST(serialize_tcp_auth);
    RUN_TEST(serialize_tcp_join);
    RUN_TEST(serialize_tcp_msg);
    RUN_TEST(serialize_tcp_err);
    RUN_TEST(serialize_tcp_bye);

    RUN_TEST(deserialize_tcp_reply_ok);
    RUN_TEST(deserialize_tcp_reply_nok);
    RUN_TEST(deserialize_tcp_auth);
    RUN_TEST(deserialize_tcp_join);
    RUN_TEST(deserialize_tcp_msg);
    RUN_TEST(deserialize_tcp_err);
    RUN_TEST(deserialize_tcp_bye);
}
