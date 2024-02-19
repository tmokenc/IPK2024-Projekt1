#include "udp.h"

void udp_auth(Client *conn) {
    (void)conn;
    // TODO
}


String udp_receive(Client *conn) {
    (void)conn;

    String str = string_new();

    // TODO
    
    return str;
}

void udp_send(Client *conn, String *str) {
    (void)conn;
    (void)str;
    // TODO
}

void udp_disconnect(Client *conn) {
    (void)conn;
    // TODO
}

