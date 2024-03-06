#include "../lib/greatest.h"
#include "../src/payload.h"
#include "../src/bytes.h"
#include <string.h>

Payload PAYLOAD;
Bytes BUFFER;

SUITE(payload);

TEST new(void) {
    PAYLOAD = payload_new(PayloadType_Bye, NULL);
    ASSERT_EQ(PAYLOAD.type, PayloadType_Bye);
    ASSERT_EQ(PAYLOAD.id, 0);

    PAYLOAD = payload_new(PayloadType_Auth, NULL);
    ASSERT_EQ(PAYLOAD.type, PayloadType_Auth);
    ASSERT_EQ(PAYLOAD.id, 1);

    PAYLOAD = payload_new(PayloadType_Confirm, NULL);
    ASSERT_EQ(PAYLOAD.type, PayloadType_Confirm);
    ASSERT_EQ(PAYLOAD.id, 2);

    PASS();
}

GREATEST_SUITE(payload) {
    RUN_TEST(new);
}
