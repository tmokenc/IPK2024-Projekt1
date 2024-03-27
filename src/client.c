/**
 * @file client.c
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 27/03/2024
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
#include "time.h"
#include "bit_field.h"

struct current_payload {
    Payload payload;
    bool got_ack;
    bool executed;
    int retry_count;
    Timestamp timestamp;
};

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
void client_send(PayloadType, PayloadData *);

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
BitField RECEIVED_ID;
DisplayName DISPLAY_NAME;
struct current_payload CURRENT_PAYLOAD;

void handle_sigint(int sig) { 
    logfmt("Get signal %u", sig);
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

    while (!(STATE == State_End && CURRENT_PAYLOAD.got_ack)) {
        int timeout = -1;
        int nof_fds = 2;

        if (STATE == State_Auth) {
            nof_fds = 1;
        }

        if (!CURRENT_PAYLOAD.got_ack) {
            timeout = CONNECTION.args.udp_timeout - timestamp_elapsed(CURRENT_PAYLOAD.timestamp);
            /// Very rare case but better safe than sorry
            if (timeout < 0) timeout = 0;
            nof_fds = 1;
        }

        logfmt("Polling with timeout of %d ms", timeout);
        int ret = poll(poll_fds, nof_fds, timeout);

        if (ret < 0) {
            // GOT ERROR
            STATE = State_End;
            if (STATE == State_Start) {
                break;
            }
            client_send(PayloadType_Bye, NULL);
            continue;
        }

        if (ret == 0) {
            /// TIMEOUT
            
            if (++CURRENT_PAYLOAD.retry_count > CONNECTION.args.udp_retransmissions) {
                /// Consider disconnected
                STATE = State_End;
                break;
            }

            CONNECTION.send(&CONNECTION, CURRENT_PAYLOAD.payload);
            CURRENT_PAYLOAD.timestamp = timestamp_now();
            continue;
        }

        if (poll_fds[0].revents & POLLIN) {
            // GOT MESSAGE FROM SERVER
            client_handle_socket();
        }

        if (nof_fds == 2 && poll_fds[1].revents & POLLIN) {
            // GOT USER INPUT
            client_handle_input();
        }

        if (CURRENT_PAYLOAD.got_ack && !CURRENT_PAYLOAD.executed) {

        }

        log("Done a event loop");
    }

    client_shutdown();
}

void client_init(Args args) {
    log("Initializing client");
    command_setup();
    signal(SIGINT, handle_sigint); 
    RECEIVED_ID = bit_field_new();

    if (get_error()) return;

    CONNECTION = connection_init(args);
    CONNECTION.connect(&CONNECTION);
    CURRENT_PAYLOAD.got_ack = true;
    log("Initialized");
}

void client_shutdown() {
    log("Shutting down");
    bit_field_free(&RECEIVED_ID);
    connection_close(&CONNECTION);
    command_clean_up();
}

void client_handle_socket() {
    log("Start handling incoming packet");
    Payload payload = CONNECTION.receive(&CONNECTION);

    if (get_error()) {
        // TODO
    }

    logfmt("Got payload type %d", payload.type);

    /// Wait for ack first
    if (!CURRENT_PAYLOAD.got_ack && payload.type != PayloadType_Confirm) {
        return;
    }

    if (payload.type != PayloadType_Confirm) {
        if (CONNECTION.args.mode == Mode_UDP && bit_field_contains(&RECEIVED_ID, payload.id)) {
            return;
        } else {
            bit_field_insert(&RECEIVED_ID, payload.id);
        }
    }

    switch (payload.type) {
        case PayloadType_Confirm:
            logfmt("Confirming %u", payload.id);
            if (payload.id == CURRENT_PAYLOAD.payload.id) {
                CURRENT_PAYLOAD.got_ack = true;
                CURRENT_PAYLOAD.retry_count = 0;
                log("Confirmed");
            }
            return;

        case PayloadType_Auth:
        case PayloadType_Join: {
            /// Error state, send the ERR then go to the End state immediately.
            PayloadData data = {0};
            memcpy(data.err.display_name, DISPLAY_NAME, DISPLAY_NAME_LEN + 1);
            strcpy((void *)data.err.message_content, "Received malformed payload");
            client_send(PayloadType_Err, &data);
            STATE = State_End;
            break;
        }

        case PayloadType_Reply:
            if (STATE != State_Auth && STATE != State_Open) {
                return;
            }

            if (payload.data.reply.result) {
                fprintf(stderr, "Success: ");
                STATE = State_Open;
            } else {
                fprintf(stderr, "Failure: ");
                if (STATE == State_Auth) STATE = State_Start;
            }

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
    log("Start handling user input");
    Bytes buffer = bytes_new();
    int result = readLineStdin(&buffer);

    if (result == EOF) {
        // shutdown
        client_send(PayloadType_Bye, NULL);
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
            strcpy((void *)data.message.message_content, (void *)cmd.data.message);

            client_send(PayloadType_Message, &data);
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

            client_send(PayloadType_Auth, &data);
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

            client_send(PayloadType_Join, &data);
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
            client_send(PayloadType_Bye, NULL);
            break;

    }
}

void client_send(PayloadType type, PayloadData *data) {
    CURRENT_PAYLOAD.payload = payload_new(type, data);
    CONNECTION.send(&CONNECTION, CURRENT_PAYLOAD.payload);
    
    if (get_error()) {
        // TODO
    }
    
    CURRENT_PAYLOAD.got_ack = CONNECTION.args.mode == Mode_TCP;
    CURRENT_PAYLOAD.timestamp = timestamp_now();
}
