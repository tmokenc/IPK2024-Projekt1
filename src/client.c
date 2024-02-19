#include "client.h"
#include "udp.h"
#include "tcp.h"

Client client_connect_udp(Args args) {
    Client conn;
    // TODO
    return conn;
}

Client client_connect_tcp(Args args) {
    Client conn;
    // TODO
    return conn;
}

/// return how many byte read
String client_receive(Client *conn) {
    return (conn->receive)(conn);
}

void client_send(Client *conn, String *str) {
    (conn->send)(conn, str);
}

void disconnect(Client *conn) {
    (conn->disconnect)(conn);
}

