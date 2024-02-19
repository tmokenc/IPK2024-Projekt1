#ifndef CONNECTION_H
#define CONNECTION_H

#include "args.h"
#include "string.h"
#include <stdio.h>

struct Connection;

typedef void (*connect_func)(Args);
typedef String (*receive_func)(struct Connection *);
typedef void (*send_func)(struct Connection *, String *);
typedef void (*disconnect_func)(struct Connection *);

typedef struct Connection {
    int sockfd;
    connect_func connect;
    receive_func receive;
    send_func send;
    disconnect_func disconnect;
} Connection;

Connection connection_connect_udp(Args args);
Connection connection_connect_tcp(Args args);

String connection_receive(Connection *conn);

void connection_send(Connection *conn, String *str);

void disconnect(Connection *conn);

#endif
