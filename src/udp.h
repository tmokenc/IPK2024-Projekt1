#ifndef UDP_H
#define UDP_H

#include "client.h"

void udp_auth(Client *conn);
String udp_receive(Client *conn);
void udp_send(Client *conn, String *str);
void udp_disconnect(Client *conn);

#endif
