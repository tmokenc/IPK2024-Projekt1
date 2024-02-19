#ifndef TCP_H
#define TCP_H

#include "connection.h"

void tcp_auth(Connection *conn);
String tcp_receive(Connection *conn);
void tcp_send(Connection *conn, String *str);
void tcp_disconnect(Connection *conn);

#endif
