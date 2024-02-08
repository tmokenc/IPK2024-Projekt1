#ifndef CONNECTION_H
#define CONNECTION_H

#include "args.h"
#include <stdio.h>

typedef struct {
    int sockfd;
} Connection;

Connection connect(Args args);

/// return how many byte read
int connection_receive(Connection *conn, char *buf, int buf_len);

void connection_send(Connection *conn, char *buf, int buf_len);

void disconnect(Connection *connection);

#endif
