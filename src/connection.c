/**
 * @file connection.c
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 03/03/2024
 * @brief Implementation of the connection.h
 */

#include "connection.h"
#include "udp.h"
#include "tcp.h"
#include "error.h"
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

Connection connection_init(Args args) {
    Connection conn;
    conn.args = args;

    int family = AF_INET;
    int type = 0;

    switch (args.mode) {
        case Mode_UDP:
            type = SOCK_DGRAM;
            conn.connect = udp_connect;
            conn.send = udp_send;
            conn.receive = udp_receive;
            conn.disconnect = udp_disconnect;
            break;
        case Mode_TCP:
            type = SOCK_STREAM;
            conn.connect = tcp_connect;
            conn.send = tcp_send;
            conn.receive = tcp_receive;
            conn.disconnect = tcp_disconnect;
            break;
    }

    conn.sockfd = socket(family, type, 0);

    if (conn.sockfd <= 0) {
        fprintf(stderr, "ERR: cannot create socket\n");
        set_error(Error_Socket);
        return conn;
    }

    int flags = fcntl(conn.sockfd, F_GETFL, 0);
    if (fcntl(conn.sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
        fprintf(stderr, "ERR: cannot set socket to be non-blocking\n");
        set_error(Error_Socket);
        return conn;
    }


    struct addrinfo hints;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = family;
    hints.ai_socktype = type;

    // the second argument takes port number as a string, not a number,
    // because it can be any service name, not only port
    char port[6];
    sprintf(port, "%d", args.port);

    int result = getaddrinfo(args.host, port, &hints, &conn.address_info);

    if (result != 0) {
        fprintf(stderr, "ERR: Cannot get the address info of %s\n", args.host);
        set_error(Error_Connection);
        return conn;
    }

    return conn;
}

void connection_close(Connection *conn) {
    conn->disconnect(conn);
    freeaddrinfo(conn->address_info);

    if (close(conn->sockfd) == -1) {
        fprintf(stderr, "ERR: Cannot close the socket\n");
    }
}
