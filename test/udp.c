#include "greatest.h"
#include "../src//error.h"
#include "../src/payload.h"
#include "../src/udp.h"
#include "../src/bytes.h"
#include <ctype.h>
#include <string.h>

Payload UDP_PAYLOAD;
Bytes UDP_BUFFER;

static void udp_setup(void *arg) {
    UDP_BUFFER = bytes_new();
    set_error(Error_None);
    (void)arg;
}

static void udp_tear_down(void *arg) {
    memset(&UDP_PAYLOAD, 0, sizeof(Payload));
    bytes_clear(&UDP_BUFFER);
    (void)arg;
}

void udp_push_message_id(MessageID message_id) {
    bytes_push(&UDP_BUFFER, message_id >> 8);
    bytes_push(&UDP_BUFFER, message_id & 0xFF);
}

void udp_push_header(PayloadType type, MessageID message_id) {
    bytes_push(&UDP_BUFFER, type);
    udp_push_message_id(message_id);
}

void udp_push_data(uint8_t *data, size_t len) {
    bytes_push_arr(&UDP_BUFFER, data, len);
    bytes_push(&UDP_BUFFER, 0);
}

SUITE(udp);

static enum greatest_test_res udp_serialize_test(uint8_t *expect, size_t len) {
    bytes_clear(&UDP_BUFFER);
    UDP_BUFFER = udp_serialize(&UDP_PAYLOAD);
    ASSERT_FALSE(get_error());
    ASSERT_EQ(UDP_BUFFER.len, len);
    ASSERT_MEM_EQ(UDP_BUFFER.data, expect, len);
    PASS();
}

TEST udp_serialize_confirm(void) {
    uint8_t expect[3] = {0x00, 0xFA, 0xAF};

    UDP_PAYLOAD.type = PayloadType_Confirm;
    UDP_PAYLOAD.id = 0xFAAF;

    CHECK_CALL(udp_serialize_test(expect, sizeof(expect)));

    PASS();
}

TEST udp_serialize_reply(void) {
    uint8_t expect[] = {0x01, 0xFA, 0xAF, 0x01, 0xAF, 0xFA, 'H', 'e', 'l', 'l', 'o', 0};

    UDP_PAYLOAD.type = PayloadType_Reply;
    UDP_PAYLOAD.id = 0xFAAF;
    UDP_PAYLOAD.data.reply.ref_message_id = 0xAFFA;
    UDP_PAYLOAD.data.reply.result = true;
    strcpy((void *)UDP_PAYLOAD.data.reply.message_content, "Hello");

    CHECK_CALL(udp_serialize_test(expect, sizeof(expect)));

    UDP_PAYLOAD.data.reply.result = false;
    expect[3] = 0x00;

    CHECK_CALL(udp_serialize_test(expect, sizeof(expect)));

    PASS();
}

TEST udp_serialize_auth(void) {
    uint8_t expect[] = {
        0x02, 
        0xFA, 0xAF, 
        't', 'm', 'o', 'k', 'e', 'n', 'c', 0,
        't', 'o', 'm', 'o', 'k', 'a', 0,
        'H', 'e', 'l', 'l', 'o', 0
    };

    UDP_PAYLOAD.type = PayloadType_Auth;
    UDP_PAYLOAD.id = 0xFAAF;
    strcpy((void *)UDP_PAYLOAD.data.auth.username, "tmokenc");
    strcpy((void *)UDP_PAYLOAD.data.auth.display_name, "tomoka");
    strcpy((void *)UDP_PAYLOAD.data.auth.secret, "Hello");

    CHECK_CALL(udp_serialize_test(expect, sizeof(expect)));

    PASS();
}

TEST udp_serialize_join(void) {
    uint8_t expect[] = {
        0x03, 
        0xFA, 0xAF, 
        'g', 'e', 'n', 'e', 'r', 'a', 'l', 0,
        't', 'o', 'm', 'o', 'k', 'a', 0,
    };

    UDP_PAYLOAD.type = PayloadType_Join;
    UDP_PAYLOAD.id = 0xFAAF;
    strcpy((void *)UDP_PAYLOAD.data.join.channel_id, "general");
    strcpy((void *)UDP_PAYLOAD.data.join.display_name, "tomoka");

    CHECK_CALL(udp_serialize_test(expect, sizeof(expect)));

    PASS();
}
TEST udp_serialize_msg(void) {
    uint8_t expect[] = {
        0x04, 
        0xFA, 0xAF, 
        't', 'o', 'm', 'o', 'k', 'a', 0,
        'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', 0,
    };

    UDP_PAYLOAD.type = PayloadType_Message;
    UDP_PAYLOAD.id = 0xFAAF;
    strcpy((void *)UDP_PAYLOAD.data.message.display_name, "tomoka");
    strcpy((void *)UDP_PAYLOAD.data.message.message_content, "Hello World");

    CHECK_CALL(udp_serialize_test(expect, sizeof(expect)));

    PASS();
}
TEST udp_serialize_err(void) {
    uint8_t expect[] = {
        0xFE, 
        0xFA, 0xAF, 
        't', 'o', 'm', 'o', 'k', 'a', 0,
        'W', 'r', 'o', 'n', 'g', 0
    };

    UDP_PAYLOAD.type = PayloadType_Err;
    UDP_PAYLOAD.id = 0xFAAF;
    strcpy((void *)UDP_PAYLOAD.data.message.display_name, "tomoka");
    strcpy((void *)UDP_PAYLOAD.data.message.message_content, "Wrong");

    CHECK_CALL(udp_serialize_test(expect, sizeof(expect)));

    PASS();
}

TEST udp_serialize_bye(void) {
    uint8_t expect[] = { 0xFF, 0xFA, 0xAF };

    UDP_PAYLOAD.type = PayloadType_Bye;
    UDP_PAYLOAD.id = 0xFAAF;

    CHECK_CALL(udp_serialize_test(expect, sizeof(expect)));
    PASS();
}
 
TEST udp_deserialize_confirm(void) {
    uint8_t data[3] = {0x00, 0xFA, 0xAF};
    bytes_push_arr(&UDP_BUFFER, data, sizeof(data));

    UDP_PAYLOAD = udp_deserialize(UDP_BUFFER);

    ASSERT_FALSE(get_error());
    ASSERT_EQ(UDP_PAYLOAD.type, PayloadType_Confirm);
    ASSERT_EQ(UDP_PAYLOAD.id, 0xFAAF);

    PASS();
}

TEST udp_deserialize_reply(void) {
    uint8_t data[] = {0x01, 0xFA, 0xAF, 0x01, 0xAF, 0xFA, 'H', 'e', 'l', 'l', 'o', 0};
    bytes_push_arr(&UDP_BUFFER, data, sizeof(data));

    UDP_PAYLOAD = udp_deserialize(UDP_BUFFER);

    ASSERT_FALSE(get_error());
    ASSERT_EQ(UDP_PAYLOAD.type, PayloadType_Reply);
    ASSERT_EQ(UDP_PAYLOAD.id, 0xFAAF);
    ASSERT_EQ(UDP_PAYLOAD.data.reply.ref_message_id, 0xAFFA);
    ASSERT(UDP_PAYLOAD.data.reply.result);
    ASSERT_STR_EQ(UDP_PAYLOAD.data.reply.message_content, "Hello");

    data[3] = 0x00;
    bytes_clear(&UDP_BUFFER);
    bytes_push_arr(&UDP_BUFFER, data, sizeof(data));
    UDP_PAYLOAD = udp_deserialize(UDP_BUFFER);

    ASSERT_FALSE(get_error());
    ASSERT_EQ(UDP_PAYLOAD.type, PayloadType_Reply);
    ASSERT_EQ(UDP_PAYLOAD.id, 0xFAAF);
    ASSERT_EQ(UDP_PAYLOAD.data.reply.ref_message_id, 0xAFFA);
    ASSERT_FALSE(UDP_PAYLOAD.data.reply.result);
    ASSERT_STR_EQ(UDP_PAYLOAD.data.reply.message_content, "Hello");

    PASS();
}

TEST udp_deserialize_auth(void) {
    uint8_t data[] = {
        0x02, 
        0xFA, 0xAF, 
        't', 'm', 'o', 'k', 'e', 'n', 'c', 0,
        't', 'o', 'm', 'o', 'k', 'a', 0,
        'H', 'e', 'l', 'l', 'o', 0
    };

    bytes_push_arr(&UDP_BUFFER, data, sizeof(data));
    UDP_PAYLOAD = udp_deserialize(UDP_BUFFER);

    ASSERT_FALSE(get_error());
    ASSERT_EQ(UDP_PAYLOAD.type, PayloadType_Auth);
    ASSERT_EQ(UDP_PAYLOAD.id, 0xFAAF);
    ASSERT_STR_EQ(UDP_PAYLOAD.data.auth.username, "tmokenc");
    ASSERT_STR_EQ(UDP_PAYLOAD.data.auth.display_name, "tomoka");
    ASSERT_STR_EQ(UDP_PAYLOAD.data.auth.secret, "Hello");

    PASS();
}

TEST udp_deserialize_join(void) {
    uint8_t data[] = {
        0x03, 
        0xFA, 0xAF, 
        'g', 'e', 'n', 'e', 'r', 'a', 'l', 0,
        't', 'o', 'm', 'o', 'k', 'a', 0,
    };

    bytes_push_arr(&UDP_BUFFER, data, sizeof(data));
    UDP_PAYLOAD = udp_deserialize(UDP_BUFFER);

    ASSERT_FALSE(get_error());
    ASSERT_EQ(UDP_PAYLOAD.type, PayloadType_Join);
    ASSERT_EQ(UDP_PAYLOAD.id, 0xFAAF);
    ASSERT_STR_EQ(UDP_PAYLOAD.data.join.channel_id, "general");
    ASSERT_STR_EQ(UDP_PAYLOAD.data.join.display_name, "tomoka");

    PASS();

}

TEST udp_deserialize_msg(void) {
    uint8_t data[] = {
        0x04, 
        0xFA, 0xAF, 
        't', 'o', 'm', 'o', 'k', 'a', 0,
        'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', 0,
    };

    bytes_push_arr(&UDP_BUFFER, data, sizeof(data));
    UDP_PAYLOAD = udp_deserialize(UDP_BUFFER);

    ASSERT_FALSE(get_error());
    ASSERT_EQ(UDP_PAYLOAD.type, PayloadType_Message);
    ASSERT_EQ(UDP_PAYLOAD.id, 0xFAAF);
    ASSERT_STR_EQ(UDP_PAYLOAD.data.message.display_name, "tomoka");
    ASSERT_STR_EQ(UDP_PAYLOAD.data.message.message_content, "Hello World");

    PASS();
}

TEST udp_deserialize_err(void) {
    uint8_t data[] = {
        0xFE, 
        0xFA, 0xAF, 
        't', 'o', 'm', 'o', 'k', 'a', 0,
        'W', 'r', 'o', 'n', 'g', 0
    };

    bytes_push_arr(&UDP_BUFFER, data, sizeof(data));
    UDP_PAYLOAD = udp_deserialize(UDP_BUFFER);

    ASSERT_FALSE(get_error());
    ASSERT_EQ(UDP_PAYLOAD.type, PayloadType_Err);
    ASSERT_EQ(UDP_PAYLOAD.id, 0xFAAF);
    ASSERT_STR_EQ(UDP_PAYLOAD.data.message.display_name, "tomoka");
    ASSERT_STR_EQ(UDP_PAYLOAD.data.message.message_content, "Wrong");

    PASS();
}

TEST udp_deserialize_bye(void) {
    uint8_t data[] = { 0xFF, 0xFA, 0xAF };

    bytes_push_arr(&UDP_BUFFER, data, sizeof(data));
    UDP_PAYLOAD = udp_deserialize(UDP_BUFFER);

    ASSERT_FALSE(get_error());
    ASSERT_EQ(UDP_PAYLOAD.type, PayloadType_Bye);
    ASSERT_EQ(UDP_PAYLOAD.id, 0xFAAF);

    PASS();
}

static greatest_test_res udp_deserialize_error(char *msg, uint8_t *data, size_t len) {
    bytes_push_arr(&UDP_BUFFER, data, len);
    UDP_PAYLOAD = udp_deserialize(UDP_BUFFER);
    ASSERTm(msg, get_error());
    // clean up
    set_error(Error_None);
    bytes_clear(&UDP_BUFFER);

    PASS();
}

TEST udp_deserialize_invalid_secret(void) {
    uint8_t empty[] = {
        0x02, 
        0xFA, 0xAF, 
        't', 'm', 'o', 'k', 'e', 'n', 'c', 0,
        't', 'o', 'm', 'o', 'k', 'a', 0,
    };

    CHECK_CALL(udp_deserialize_error("empty", empty, sizeof(empty)));
    
    uint8_t invalid[] = {
        0x02, 
        0xFA, 0xAF, 
        't', 'm', 'o', 'k', 'e', 'n', 'c', 0,
        't', 'o', 'm', 'o', 'k', 'a', 0,
        19, 0
    };

    CHECK_CALL(udp_deserialize_error("contains invalid characters", invalid, sizeof(invalid)));

    PASS();
}

TEST udp_deserialize_invalid_username(void) {
    uint8_t empty[] = {
        0x02, 
        0xFA, 0xAF, 
        0,
        't', 'o', 'm', 'o', 'k', 'a', 0,
        'H', 'e', 'l', 'l', 'o', 0
    };

    CHECK_CALL(udp_deserialize_error("empty", empty, sizeof(empty)));

    uint8_t invalid[] = {
        0x02, 
        0xFA, 0xAF, 
        2, 0,
        't', 'o', 'm', 'o', 'k', 'a', 0,
        'H', 'e', 'l', 'l', 'o', 0
    };

    CHECK_CALL(udp_deserialize_error("contains invalid characters", invalid, sizeof(invalid)));
    PASS();
}

TEST udp_deserialize_invalid_channel_id(void) {
    uint8_t empty[] = {
        0x03, 
        0xFA, 0xAF, 
        't', 'o', 'm', 'o', 'k', 'a', 0,
    };

    CHECK_CALL(udp_deserialize_error("empty", empty, sizeof(empty)));

    uint8_t invalid[] = {
        0x03, 
        0xFA, 0xAF, 
        1, 0,
        't', 'o', 'm', 'o', 'k', 'a', 0,
    };

    CHECK_CALL(udp_deserialize_error("contains invalid characters", invalid, sizeof(invalid)));

    PASS();
}

TEST udp_deserialize_invalid_display_name(void) {
    uint8_t empty_display_name[] = {
        0xFE, 
        0xFA, 0xAF, 
        0,
        't', 'o', 'm', 'o', 'k', 'a', 0,
    };

    CHECK_CALL(udp_deserialize_error("empty", empty_display_name, sizeof(empty_display_name)));

    uint8_t invalid_chars[] = {
        0xFE,
        0xFA, 0xAF,
        0x19, 0,
        't', 'o', 'm', 'o', 'k', 'a', 0,
    };

    CHECK_CALL(udp_deserialize_error("contains invalid characters", invalid_chars, sizeof(invalid_chars)));

    PASS();
}

TEST udp_deserialize_invalid_message_content(void) {
    uint8_t empty_content[] = {
        0xFE, 
        0xFA, 0xAF, 
        't', 'o', 'm', 'o', 'k', 'a', 0,
        0,
    };

    CHECK_CALL(udp_deserialize_error("empty", empty_content, sizeof(empty_content)));

    uint8_t invalid_chars[] = {
        0xFE, 
        0xFA, 0xAF, 
        't', 'o', 'm', 'o', 'k', 'a', 0,
        0x19, 0,
    };

    CHECK_CALL(udp_deserialize_error("contains invalid characters", invalid_chars, sizeof(invalid_chars)));

    PASS();
}

GREATEST_SUITE(udp) {
    GREATEST_SET_SETUP_CB(udp_setup, NULL);
    GREATEST_SET_TEARDOWN_CB(udp_tear_down, NULL);

    RUN_TEST(udp_serialize_confirm);
    RUN_TEST(udp_serialize_reply);
    RUN_TEST(udp_serialize_auth);
    RUN_TEST(udp_serialize_join);
    RUN_TEST(udp_serialize_msg);
    RUN_TEST(udp_serialize_err);
    RUN_TEST(udp_serialize_bye);

    RUN_TEST(udp_deserialize_confirm);
    RUN_TEST(udp_deserialize_reply);
    RUN_TEST(udp_deserialize_auth);
    RUN_TEST(udp_deserialize_join);
    RUN_TEST(udp_deserialize_msg);
    RUN_TEST(udp_deserialize_err);
    RUN_TEST(udp_deserialize_bye);

    RUN_TEST(udp_deserialize_invalid_secret);
    RUN_TEST(udp_deserialize_invalid_username);
    RUN_TEST(udp_deserialize_invalid_channel_id);
    RUN_TEST(udp_deserialize_invalid_display_name);
    RUN_TEST(udp_deserialize_invalid_message_content);
}
