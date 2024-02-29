#ifndef CLIENT_H
#define CLIENT_H

#include "args.h"
#include "string.h"
#include <stdint.h>
#include <stdio.h>
#include "connection.h"

struct Client;

typedef struct Client {
    Connection connection;
} Client;

Client client_init(Args args);

void client_start(Client *client);

void shutdown(Client *client);

#endif
