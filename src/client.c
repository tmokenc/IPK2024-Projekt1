#include "client.h"

Client client_init(Args args) {
    Client client;
    client.connection = connection_init(args);
    return client;
}

void client_start(Client *client) {
    (void)client;
}

void client_shutdown(Client *client) {
    Payload payload = payload_new(PayloadType_Bye, NULL);
    client->connection.send(&client->connection, payload);
    client->connection.disconnect(&client->connection);
}
