
#include "greatest.h"

SUITE(payload);

TEST new(void) {
    SKIPm("TODO");
}

TEST serialize_tcp(void) {
    SKIPm("TODO");
}

TEST deserialize_tcp(void) {
    SKIPm("TODO");
}

TEST serialize_udp(void) {
    SKIPm("TODO");
}

TEST deserialize_udp(void) {
    SKIPm("TODO");
}

GREATEST_SUITE(payload) {
    RUN_TEST(new);
    RUN_TEST(serialize_tcp);
    RUN_TEST(deserialize_tcp);
    RUN_TEST(serialize_udp);
    RUN_TEST(deserialize_udp);

}
