#ifndef TCP_H
#define TCP_H

#include "client.h"

void tcp_auth(Client *conn);
String tcp_receive(Client *conn);
void tcp_send(Client *conn, String *str);
void tcp_disconnect(Client *conn);

#endif
