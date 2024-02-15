#include "connection.h"

Connection connect(Args args) {
    Connection conn = {0};

    (void)args;
    // TODO

    return conn;
}

/// return how many byte read
String connection_receive(Connection *conn) {
    String str = string_new();

    (void)conn;
    // TODO
    
    return str;
}

void connection_send(Connection *conn, String *str) {
    (void)conn;
    (void)str;
    // TODO
}

void disconnect(Connection *conn) {
    (void)conn;
    // TODO
}

