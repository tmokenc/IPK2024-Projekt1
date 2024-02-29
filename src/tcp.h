#ifndef TCP_H
#define TCP_H

#include "connection.h"
#include <stdint.h>

int tcp_connect(char *host, uint16_t port);
String tcp_receive(Connection *conn);
void tcp_disconnect(Connection *conn);

void tcp_err(Connection *);
void tcp_confirm(Connection *);
void tcp_reply(Connection *, bool, MessageContent);
void tcp_auth(Connection *, Username, DisplayName, Secret);
void tcp_join(Connection *, ChannelID, DisplayName);
void tcp_msg(Connection *, DisplayName, MessageContent);
void tcp_bye(Connection *);

#endif
