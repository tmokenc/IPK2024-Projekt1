#ifndef UDP_H
#define UDP_H

#include "connection.h"

void udp_auth(Connection *conn);
String udp_receive(Connection *conn);
void udp_send(Connection *conn, String *str);
void udp_disconnect(Connection *conn);

#endif
