#include "tcp.h"

void tcp_auth(Connection *conn) {
    (void)conn;
    // TODO
}


String tcp_receive(Connection *conn) {
    (void)conn;

    String str = string_new();

    // TODO
    
    return str;
}

void tcp_send(Connection *conn, String *str) {
    (void)conn;
    (void)str;
    // TODO
}

void tcp_disconnect(Connection *conn) {
    (void)conn;
    // TODO
}

