#include "tcp.h"

void tcp_auth(Client *conn) {
    (void)conn;
    // TODO
}


String tcp_receive(Client *conn) {
    (void)conn;

    String str = string_new();

    // TODO
    
    return str;
}

void tcp_send(Client *conn, String *str) {
    (void)conn;
    (void)str;
    // TODO
}

void tcp_disconnect(Client *conn) {
    (void)conn;
    // TODO
}

