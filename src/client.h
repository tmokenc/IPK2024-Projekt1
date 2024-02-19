#ifndef CLIENT_H
#define CLIENT_H

#include "args.h"
#include "string.h"
#include <stdio.h>

struct Client;

typedef void (*connect_func)(Args);
typedef String (*receive_func)(struct Client *);
typedef void (*send_func)(struct Client *, String *);
typedef void (*disconnect_func)(struct Client *);

typedef struct Client {
    int sockfd;
    connect_func connect;
    receive_func receive;
    send_func send;
    disconnect_func disconnect;
} Client;

Client client_connect_udp(Args args);
Client client_connect_tcp(Args args);

String client_receive(Client *conn);

void client_send(Client *conn, String *str);

void disconnect(Client *conn);

#endif
