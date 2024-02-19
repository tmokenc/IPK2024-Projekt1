#include "connection.h"
#include "udp.h"
#include "tcp.h"

Connection connection_connect_udp(Args args) {
    Connection conn;
    // TODO
    return conn;
}

Connection connection_connect_tcp(Args args) {
    Connection conn;
    // TODO
    return conn;
}

/// return how many byte read
String connection_receive(Connection *conn) {
    return (conn->receive)(conn);
}

void connection_send(Connection *conn, String *str) {
    (conn->send)(conn, str);
}

void disconnect(Connection *conn) {
    (conn->disconnect)(conn);
}

