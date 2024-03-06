#include "greatest.h"
#include "../src//error.h"
#include "../src/payload.h"
#include "../src/bytes.h"
#include <string.h>

Payload UDP_PAYLOAD;
Bytes UDP_BUFFER;

static void udp_setup(void *arg) {
    UDP_BUFFER = bytes_new();
    set_error(Error_None);
    (void)arg;
}

static void udp_tear_down(void *arg) {
    bytes_free(&UDP_BUFFER);
    (void)arg;
}


SUITE(udp);

TEST serialize_udp_confirm(void) { SKIPm("TODO"); }
TEST serialize_udp_reply(void) { SKIPm("TODO"); }
TEST serialize_udp_auth(void) { SKIPm("TODO"); }
TEST serialize_udp_join(void) { SKIPm("TODO"); }
TEST serialize_udp_msg(void) { SKIPm("TODO"); }
TEST serialize_udp_err(void) { SKIPm("TODO"); }
TEST serialize_udp_bye(void) { SKIPm("TODO"); }
 
TEST deserialize_udp_confirm(void) { SKIPm("TODO"); }
TEST deserialize_udp_reply(void) { SKIPm("TODO"); }
TEST deserialize_udp_auth(void) { SKIPm("TODO"); }
TEST deserialize_udp_join(void) { SKIPm("TODO"); }
TEST deserialize_udp_msg(void) { SKIPm("TODO"); }
TEST deserialize_udp_err(void) { SKIPm("TODO"); }
TEST deserialize_udp_bye(void) { SKIPm("TODO"); }

GREATEST_SUITE(udp) {
    GREATEST_SET_SETUP_CB(udp_setup, NULL);
    GREATEST_SET_TEARDOWN_CB(udp_tear_down, NULL);

    RUN_TEST(serialize_udp_confirm);
    RUN_TEST(serialize_udp_reply);
    RUN_TEST(serialize_udp_auth);
    RUN_TEST(serialize_udp_join);
    RUN_TEST(serialize_udp_msg);
    RUN_TEST(serialize_udp_err);
    RUN_TEST(serialize_udp_bye);

    RUN_TEST(deserialize_udp_confirm);
    RUN_TEST(deserialize_udp_reply);
    RUN_TEST(deserialize_udp_auth);
    RUN_TEST(deserialize_udp_join);
    RUN_TEST(deserialize_udp_msg);
    RUN_TEST(deserialize_udp_err);
    RUN_TEST(deserialize_udp_bye);
}
