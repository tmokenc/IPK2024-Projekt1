#ifndef CONNECTION_H
#define CONNECTION_H

#include "args.h"
#include "string.h"
#include <stdio.h>

typedef struct {
    int sockfd;
} Connection;

Connection connect(Args args);

/// return how many byte read
String connection_receive(Connection *conn);

void connection_send(Connection *conn, String *str);

void disconnect(Connection *conn);

#endif
