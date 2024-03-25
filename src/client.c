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
#include <unistd.h>
#include <signal.h> 
#include "connection.h"
#include "payload.h"

enum state {
    State_Start,
    State_Auth,
    State_Open,
    State_Error,
    State_End,
    State_EndWithoutBye,
};

void client_init(Args);
void client_shutdown();
void client_handle_input();
void client_handle_socket();

char *CHAT_HELP_MESSAGE = 
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

enum state STATE = State_Start;
PayloadType LAST_PAYLOAD_TYPE;
Connection CONNECTION;
DisplayName DISPLAY_NAME;

void handle_sigint(int sig) { 
    (void)sig;

    if (STATE == State_Start) {
        STATE = State_EndWithoutBye;
    } else {
        STATE = State_End;
    }
} 

void client_run(Args args) {
    client_init(args);

    if (get_error()) return;

    struct pollfd poll_fds[2] = {0};
    
    poll_fds[0].fd = CONNECTION.sockfd;
    poll_fds[0].events = POLLIN;

    poll_fds[1].fd = STDIN_FILENO;
    poll_fds[1].events = POLLIN;

    while (STATE != State_End && STATE != State_EndWithoutBye) {
        int timeout = CONNECTION.next_timeout(&CONNECTION);
        int nof_fds = 2;

        if (STATE == State_Auth || CONNECTION.state == ConnectionState_WaitingAck) {
            nof_fds = 1;
        }

        int ret = poll(poll_fds, nof_fds, timeout);

        if (ret <= 0) {
            // TODO
            continue;
        }

        if (poll_fds[0].revents & POLLIN) {
            // GOT MESSAGE FROM SERVER
            client_handle_socket();
        }

        if (poll_fds[1].revents & POLLIN) {
            // GOT USER INPUT
            client_handle_input();
        }
    }

    client_shutdown();
}

void client_init(Args args) {
    command_setup();
    signal(SIGINT, handle_sigint); 
    CONNECTION = connection_init(args);
    CONNECTION.connect(&CONNECTION);
}

void client_shutdown() {
    if (STATE != State_EndWithoutBye) {
        struct pollfd poll_fds = {0};
        
        poll_fds.fd = CONNECTION.sockfd;
        poll_fds.events = POLLIN;

        Payload payload = payload_new(PayloadType_Bye, NULL);
        CONNECTION.send(&CONNECTION, payload);

        // Graceful shutdown. Wait for the BYE message to be delivered
        while (CONNECTION.state == ConnectionState_WaitingAck) {
            int timeout = CONNECTION.next_timeout(&CONNECTION);
            poll(&poll_fds, 1, timeout);
        }
    }

    connection_close(&CONNECTION);
    command_clean_up();
}

void client_handle_socket() {
    Payload payload = CONNECTION.receive(&CONNECTION);

    if (get_error()) {
        // TODO
    }

    switch (payload.type) {
        case PayloadType_Confirm:
            // Skip this, it is handled by the connection itself
            break;
        case PayloadType_Auth:
        case PayloadType_Join:
            STATE = State_End;
            break;

        case PayloadType_Reply:
            if (STATE != State_Auth) {
                return;
            }

            if (payload.data.reply.result) {
                STATE = State_Open;
                fprintf(stderr, "Success: ");
            } else {
                STATE = State_Start;
                fprintf(stderr, "Failure: ");
            }

            CONNECTION.state = ConnectionState_Idle;
            fprintf(stderr, "%s\n", payload.data.reply.message_content);
            break;

        case PayloadType_Message:
            printf("%s: %s\n", payload.data.message.display_name, payload.data.message.message_content);
            break;

        case PayloadType_Err:
            fprintf(stderr, "ERR FROM %s: %s\n", payload.data.message.display_name, payload.data.message.message_content);
            STATE = State_End;
            break;

        case PayloadType_Bye:
            if (STATE == State_Open) {
                STATE = State_EndWithoutBye;
            } else {
                STATE = State_End;
            }

            break;
    }
}

void client_handle_input() {
    Bytes buffer = bytes_new();
    int result = readLineStdin(&buffer);

    Payload payload = {0};
    bool need_to_send = false;

    if (result == EOF) {
        // shutdown
        STATE = State_End;
        return;
    }

    if (result == 0) {
        return;
    }

    Command cmd = command_parse(bytes_get(&buffer));

    if (get_error()) {
        fprintf(stderr, "ERR: Cannot parse the input\n");
        error_clear();
        return;
    }

    switch (cmd.type) {
        case CommandType_None: {
            if (STATE != State_Open) {
                fprintf(stderr, "ERR: You have to join a channel first before sending messages. "
                                "Use /help for more information.\n");
                break;
            }

            PayloadData data = {0};
            memcpy(data.message.display_name, DISPLAY_NAME, DISPLAY_NAME_LEN + 1);
            memcpy(data.message.message_content, cmd.data.message, MESSAGE_CONTENT_LEN + 1);

            payload = payload_new(PayloadType_Message, &data);
            need_to_send = true;
            break;
        }

        case CommandType_Auth: {
            if (STATE != State_Start) {
                fprintf(stderr, "ERR: You have been authenticated. No need to do it again\n");
                break;
            }

            memcpy(DISPLAY_NAME, cmd.data.auth.display_name, DISPLAY_NAME_LEN + 1);

            PayloadData data = {0};
            memcpy(data.auth.display_name, DISPLAY_NAME, DISPLAY_NAME_LEN + 1);
            memcpy(data.auth.username, cmd.data.auth.username, USERNAME_LEN + 1);
            memcpy(data.auth.secret, cmd.data.auth.secret, SECRET_LEN + 1);

            payload = payload_new(PayloadType_Auth, &data);
            need_to_send = true;
            STATE = State_Auth;
            break;
        }

        case CommandType_Join: {
            if (STATE != State_Open) {
                fprintf(stderr, "ERR: Use /auth to authenticate first before joining a channel. "
                                "Use /help for more information.");
                break;
            }

            PayloadData data = {0};
            memcpy(data.join.display_name, DISPLAY_NAME, DISPLAY_NAME_LEN + 1);
            memcpy(data.join.channel_id, cmd.data.join.channel_id, CHANNEL_ID_LEN + 1);

            payload = payload_new(PayloadType_Join, &data);
            need_to_send = true;
            break;
        }

        case CommandType_Rename:
            memcpy(DISPLAY_NAME, cmd.data.rename.display_name, DISPLAY_NAME_LEN + 1);
            break;

        case CommandType_Help:
            printf("%s", CHAT_HELP_MESSAGE);
            break;

        case CommandType_Clear:
            printf("\033\143");
            fflush(stdout);
            break;

        case CommandType_Exit:
            STATE = State_End;
            break;

    }
    
    if (need_to_send) {
        CONNECTION.send(&CONNECTION, payload);
        if (get_error()) {
            // TODO
        }
    }
}
