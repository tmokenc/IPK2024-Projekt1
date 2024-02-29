#include "client.h"

Client client_init(Args args) {
    Client client;
    client.connection = connection_init(args);
    return client;
}


void client_run(Client *conn) {
    (void)(conn);
}

