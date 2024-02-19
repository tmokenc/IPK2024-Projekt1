#ifndef COMMAND_H
#define COMMAND_H

#include "message.h"
#include "connection.h"

void command_auth(Connection *conn, Username username, Secret secret, DisplayName name);
void command_join(Connection *conn, ChannelID channel_id);
void command_rename(Connection *conn, DisplayName name);
void command_help(Connection *conn);

#endif
