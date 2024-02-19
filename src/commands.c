#include "commands.h"

void command_auth(Connection *conn, Username username, Secret secret, DisplayName name) {
    (void)conn;
    (void)username;
    (void)secret;
    (void)name;
    // TODO
}

void command_join(Connection *conn, ChannelID channel_id) {
    (void)conn;
    (void)channel_id;
    // TODO
}

void command_rename(Connection *conn, DisplayName name) {
    (void)conn;
    (void)name;
    // TODO
}
void command_help(Connection *conn) {
    (void)conn;
    // TODO
}

