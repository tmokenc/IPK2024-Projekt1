#ifndef UDP_H
#define UDP_H

#include "connection.h"
#include <stdint.h>

int udp_connect(char *host, uint16_t port);
String udp_receive(Connection *conn);
void udp_disconnect(Connection *conn);

void udp_err(Connection *);
void udp_confirm(Connection *);
void udp_reply(Connection *, bool, MessageContent);
void udp_auth(Connection *, Username, DisplayName, Secret);
void udp_join(Connection *, ChannelID, DisplayName);
void udp_msg(Connection *, DisplayName, MessageContent);
void udp_bye(Connection *);

#endif
