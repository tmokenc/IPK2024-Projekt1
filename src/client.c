/**
 * @file client.c
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 14/03/2024
 * @brief Implementation of client.h
 */

#include "client.h"
#include "error.h"
#include "input.h"
#include "commands.h"
#include <poll.h>
#include <string.h>

extern bool SHOULD_SHUTDOWN;

enum state {
    State_Auth,
    State_Open,
    State_Error,
    State_End,
};

enum result {
    Result_Continue,
    Result_Shutdown,
};

enum result client_handle_input(Client *, enum state *);
enum result client_handle_socket(Client *, enum state *);

void client_shutdown(Client *);

char *HELP_MESSAGE = 
"IPK2024-chat: To start, use /auth to authenticate then use /join to join a channel and now you can start chatting.\n"
"Type any message up to 1400 characters long then press enter to send. "
"The message will be sent line by line\n"
"\n"
"Commands:\n"
"/auth {username} {display_name} {secret} - start the program with this, authenticated {username} using {secret}, if success, you can start chatting under {display_name}\n"
"/join {channel_id}\n"
"/rename {display_name} - use to new {display_name} instead\n"
"/help - to show this message\n"
"/clear - clear the terminal\n"
"";

Client client_init(Args args) {
    Client client;
    command_setup();
    client.connection = connection_init(args);
    return client;
}

void client_run(Client *client) {
    client->connection.connect(&client->connection);
    if (get_error()) return;

    struct pollfd poll_fds[2] = {0};
    
    poll_fds[0].fd = client->connection.sockfd;
    poll_fds[0].events = POLLIN;

    poll_fds[1].fd = 0; // filedescriptor of stdin is 0
    poll_fds[1].events = POLLIN;

    enum state state = State_Auth;
    enum result result = Result_Continue;

    while (!SHOULD_SHUTDOWN || result != Result_Continue) {
        int timeout = client->connection.next_timeout(&client->connection);
        int ret = poll(poll_fds, 2, timeout);

        if (ret <= 0) {
            continue;
            // TODO
        }

        if (poll_fds[0].revents & POLLIN) {
            // GOT MESSAGE FROM SERVER
            result = client_handle_socket(client, &state);
        }

        if (poll_fds[1].revents & POLLIN) {
            // GOT USER INPUT
            result = client_handle_input(client, &state);
        }
    }

    client_shutdown(client);
}

void client_shutdown(Client *client) {
    Payload payload = payload_new(PayloadType_Bye, NULL);
    client->connection.send(&client->connection, payload);
    connection_close(&client->connection);
    command_clean_up();
}

enum result client_handle_socket(Client *client, enum state *state) {
    (void)state;
    Payload payload = client->connection.receive(&client->connection);

    if (get_error()) {
        // TODO
    }

    switch (payload.type) {
        case PayloadType_Confirm:
            break;
        case PayloadType_Reply:
            // TODO
            break;
        case PayloadType_Auth:
            // TODO
            break;
        case PayloadType_Join:
            // TODO
            break;
        case PayloadType_Message:
            // TODO
            break;
        case PayloadType_Err:
            // TODO
            break;
        case PayloadType_Bye:
            // TODO
            break;
    }

    // TODO
    return Result_Continue;
}

enum result client_handle_input(Client *client, enum state *state) {
    Bytes buffer = bytes_new();
    int result = readLineStdin(&buffer);

    if (result == EOF) {
        // shutdown
        return Result_Shutdown;
    }

    if (result <= 0) {
        // Error...
        return Result_Shutdown;
    }

    Command cmd = command_parse(bytes_get(&buffer));

    switch (cmd.type) {
        case CommandType_None: {
            if (*state != State_Open) {
                fprintf(stderr, "You have to join a channel first before sending messages.\n"
                                "Use /help for more information.\n");
                break;
            }

            Payload payload;
            payload.type = PayloadType_Message;
            memcpy(payload.data.message.display_name, client->display_name, DISPLAY_NAME_LEN + 1);
            memcpy(payload.data.message.message_content, cmd.data.message, MESSAGE_CONTENT_LEN + 1);
            client->connection.send(&client->connection, payload);

            break;
        }

        case CommandType_Auth: {
            if (*state != State_Auth) {
                fprintf(stderr, "You have been authenticated. No need to do it again\n");
                break;
            }

            memcpy(client->display_name, cmd.data.auth.display_name, DISPLAY_NAME_LEN + 1);

            Payload payload;
            payload.type = PayloadType_Auth;
            memcpy(payload.data.auth.display_name, client->display_name, DISPLAY_NAME_LEN + 1);
            memcpy(payload.data.auth.username, cmd.data.auth.username, USERNAME_LEN + 1);
            memcpy(payload.data.auth.secret, cmd.data.auth.secret, SECRET_LEN + 1);
            client->connection.send(&client->connection, payload);

            break;
        }

        case CommandType_Join: {
            if (*state == State_Auth) {
                fprintf(stderr, "Use /auth to authenticate first before joining a channel.\n"
                                "Use /help for more information.");
                break;
            }

            Payload payload;
            payload.type = PayloadType_Join;
            memcpy(payload.data.join.display_name, client->display_name, DISPLAY_NAME_LEN + 1);
            memcpy(payload.data.join.channel_id, cmd.data.join.channel_id, CHANNEL_ID_LEN + 1);
            client->connection.send(&client->connection, payload);

            break;
        }

        case CommandType_Rename:
            memcpy(client->display_name, cmd.data.rename.display_name, DISPLAY_NAME_LEN + 1);
            break;

        case CommandType_Help:
            printf("%s", HELP_MESSAGE);
            break;

        case CommandType_Clear:
            printf("\033\143");
            break;

    }
    
    if (get_error()) {
        // TODO
    }

    return Result_Continue;
}
