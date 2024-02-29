#include "connection.h"
#include "udp.h"
#include "tcp.h"

Connection connection_init(Args args) {
    Connection conn;

    conn.sockfd = -1;

    switch (args.mode) {
        case Mode_UDP:
            conn.bye = udp_bye;
            conn.err = udp_err;
            conn.confirm = udp_confirm;
            conn.auth = udp_auth;
            conn.join = udp_join;
            conn.msg = udp_msg;
            conn.connect = udp_connect;
            conn.receive = udp_receive;
            conn.disconnect = udp_disconnect;
            break;
        case Mode_TCP:
            conn.bye = tcp_bye;
            conn.err = tcp_err;
            conn.confirm = tcp_confirm;
            conn.auth = tcp_auth;
            conn.join = tcp_join;
            conn.msg = tcp_msg;
            conn.connect = tcp_connect;
            conn.receive = tcp_receive;
            conn.disconnect = tcp_disconnect;
            break;
    }

    return conn;
}

void connection_destroy(Connection *conn) {
    (void)conn;
}

