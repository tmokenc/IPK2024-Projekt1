#include "udp.h"

void udp_auth(Connection *conn) {
    (void)conn;
    // TODO
}


String udp_receive(Connection *conn) {
    (void)conn;

    String str = string_new();

    // TODO
    
    return str;
}

void udp_send(Connection *conn, String *str) {
    (void)conn;
    (void)str;
    // TODO
}

void udp_disconnect(Connection *conn) {
    (void)conn;
    // TODO
}

