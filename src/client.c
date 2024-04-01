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
#include <sys/epoll.h>
#include <string.h>
#include <unistd.h>
#include <signal.h> 
#include "connection.h"
#include "payload.h"
#include "time.h"
#include "bit_field.h"

/// Max event of EPOLL
#define MAX_EVENT 2

struct current_payload {
    Payload payload;
    bool confirmed;
    bool executed;
    int retry_count;
    Timestamp timestamp;
};

/**
 * Client state
 */
enum state {
    State_Start,
    State_NotAuth,
    State_Auth,
    State_Open,
    State_Error,
    State_End,
};

void client_init(Args args);
void client_shutdown();
bool client_handle_timeout();
void client_handle_input();
void client_handle_socket();
void client_send(PayloadType, PayloadData *);

char *CHAT_HELP_MESSAGE = 
"IPK2024-chat: To start, use /auth to authenticate then use /join to join a channel and now you can start chatting.\n"
"Type any message up to 1400 characters long then press enter to send. "
"The message will be sent line by line\n"
"\n"
"Commands:\n"
"/auth {username} {secret} {display_name} - start the program with this, authenticated {username} using {secret}, if success, you can start chatting under {display_name}\n"
"/join {channel_id}\n"
"/rename {display_name} - use to new {display_name} instead\n"
"/help - to show this message\n"
"/clear - clear the terminal\n"
"/exit - End the chat app\n"
"";

enum state STATE = State_Start;
Connection CONNECTION;
BitField RECEIVED_ID;
DisplayName DISPLAY_NAME;
struct current_payload CURRENT_PAYLOAD;
int EPOLL_FD_SOCKET, EPOLL_FD_SOCKET_STDIN;

void handle_sigint(int sig) { 
    logfmt("Get signal %u", sig);
    (void)sig;

    if (STATE != State_Start) {
        client_send(PayloadType_Bye, NULL);
    }

    STATE = State_End;
} 

void client_run(Args args) {
    client_init(args);

    if (get_error()) {
        client_shutdown();
        return;
    }

    struct epoll_event events[MAX_EVENT];

    while (!(STATE == State_End && CURRENT_PAYLOAD.confirmed)) {
        int timeout = -1;
        int epoll_fd = EPOLL_FD_SOCKET_STDIN;

        if (STATE == State_Auth) {
            // When in state AUTH, does not need to poll for the stdin
            epoll_fd = EPOLL_FD_SOCKET;
        }

        if (!CURRENT_PAYLOAD.confirmed) {
            timeout = CONNECTION.args.udp_timeout - timestamp_elapsed(CURRENT_PAYLOAD.timestamp);
            /// Very rare case but better safe than sorry
            if (timeout < 0) timeout = 0;
            // While waiting for CONFIRM, does not need to poll for the stdin
            epoll_fd = EPOLL_FD_SOCKET;
        }

        logfmt("Polling with timeout of %d ms", timeout);
        int num_fds = epoll_wait(epoll_fd, events, MAX_EVENT, timeout);
        logfmt("Polled with %d fds", num_fds);

        if (num_fds < 0) {
            // GOT ERROR
            STATE = State_End;
            if (STATE == State_Start) {
                break;
            }

            client_send(PayloadType_Bye, NULL);
            continue;
        }

        if (num_fds == 0) {
            /// TIMEOUT
            bool should_shutdown = client_handle_timeout();
            if (should_shutdown) {
                break;
            }

            continue;
        }

        for (int i = 0; i < num_fds; i++) {
            if (events[i].data.fd == CONNECTION.sockfd) {
                // GOT MESSAGE FROM SERVER
                client_handle_socket();
            } else if (events[i].data.fd == STDIN_FILENO) {
                // GOT USER INPUT
                client_handle_input();
            }
        }

        if (STATE == State_Error && CURRENT_PAYLOAD.confirmed) {
            client_send(PayloadType_Bye, NULL);
            STATE = State_End;
        }

        log("Done a event loop");
        fflush(stdout);
        fflush(stderr);
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
    CURRENT_PAYLOAD.confirmed = true;
    log("Initialized");

    EPOLL_FD_SOCKET = epoll_create1(0);
    EPOLL_FD_SOCKET_STDIN = epoll_create1(0);

    if (EPOLL_FD_SOCKET < 0 || EPOLL_FD_SOCKET_STDIN < 0) {
        set_error(Error_Internal);
        perror("ERR: epoll_create");
        return;
    }

    // Add STDIN file descriptor to epoll
    struct epoll_event event_stdin;
    event_stdin.events = EPOLLIN;
    event_stdin.data.fd = STDIN_FILENO;
    if (epoll_ctl(EPOLL_FD_SOCKET_STDIN, EPOLL_CTL_ADD, STDIN_FILENO, &event_stdin) == -1) {
        set_error(Error_Internal);
        perror("ERR: epoll_ctl: stdin");
        return;
    }

    // Add socket file descriptor to epolls
    struct epoll_event event_socket;
    event_socket.events = EPOLLIN;
    event_socket.data.fd = CONNECTION.sockfd;
    if (epoll_ctl(EPOLL_FD_SOCKET, EPOLL_CTL_ADD, CONNECTION.sockfd, &event_socket) == -1) {
        set_error(Error_Internal);
        perror("ERR: epoll_ctl: socket");
        return;
    }

    if (epoll_ctl(EPOLL_FD_SOCKET_STDIN, EPOLL_CTL_ADD, CONNECTION.sockfd, &event_socket) == -1) {
        set_error(Error_Internal);
        perror("ERR: epoll_ctl: socket");
        return;
    }

}

void client_shutdown() {
    log("Shutting down");
    close(EPOLL_FD_SOCKET);
    close(EPOLL_FD_SOCKET_STDIN);
    bit_field_free(&RECEIVED_ID);
    connection_close(&CONNECTION);
    command_clean_up();
}

bool client_handle_timeout() {
    if (++CURRENT_PAYLOAD.retry_count > CONNECTION.args.udp_retransmissions) {
        /// Consider disconnected
        STATE = State_End;
        return true;
    }
    
    CONNECTION.send(&CONNECTION, CURRENT_PAYLOAD.payload);
    CURRENT_PAYLOAD.timestamp = timestamp_now();
    return false;
}

void client_handle_socket() {
    log("Start handling incoming packet");
    Payload payload = CONNECTION.receive(&CONNECTION);

    if (get_error() == Error_RecvFromWrongAddress) {
        // Just ignore it
        error_clear();
        return;
    }

    if (payload.type != PayloadType_Confirm) {
        if (!CURRENT_PAYLOAD.confirmed) {
            /// Wait for CONFIRM first if the last payload was not confirmed
            return;
        }

        log("Sending confirm");
        Payload confirm;
        confirm.type = PayloadType_Confirm;
        confirm.id = payload.id;
        CONNECTION.send(&CONNECTION, confirm);
    }

    if (get_error()) {
        error_clear();
        eprint("Received malformed payload");
        PayloadData data = {0};
        memcpy(data.err.display_name, DISPLAY_NAME, DISPLAY_NAME_LEN + 1);
        strcpy((void *)data.err.message_content, "Received malformed payload");
        client_send(PayloadType_Err, &data);
        STATE = State_Error;
        return;
    }


    if (payload.type != PayloadType_Confirm) {
        if (CONNECTION.args.mode == Mode_UDP && bit_field_contains(&RECEIVED_ID, payload.id)) {
            log("Received duplicated packed");
            return;
        }

        bit_field_insert(&RECEIVED_ID, payload.id);
    }

    switch (payload.type) {
        case PayloadType_Confirm:
            logfmt("Confirming %u", payload.id);
            if (payload.id == CURRENT_PAYLOAD.payload.id) {
                CURRENT_PAYLOAD.confirmed = true;
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
            STATE = State_Error;
            break;
        }

        case PayloadType_Reply:
            if (STATE != State_Auth && STATE != State_Open) {
                return;
            }

            logfmt("Got reply to %d", payload.data.reply.ref_message_id);

            if (payload.data.reply.result) {
                fprintf(stderr, "Success: ");
                STATE = State_Open;
            } else {
                fprintf(stderr, "Failure: ");
                if (STATE == State_Auth) STATE = State_NotAuth;
            }

            fprintf(stderr, "%s\n", payload.data.reply.message_content);
            fflush(stderr);
            break;

        case PayloadType_Message:
            printf("%s: %s\n", payload.data.message.display_name, payload.data.message.message_content);
            break;

        case PayloadType_Err:
            fprintf(stderr, "ERR FROM %s: %s\n", payload.data.err.display_name, payload.data.err.message_content);
            client_send(PayloadType_Bye, NULL);
            STATE = State_End;
            break;

        case PayloadType_Bye:
            if (STATE != State_Open) {
                client_send(PayloadType_Bye, NULL);
            }

            STATE = State_End;

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
        eprint("Cannot parse the input");
        error_clear();
        return;
    }

    switch (cmd.type) {
        case CommandType_None: {
            if (STATE != State_Open) {
                eprint("You have to join a channel first before sending messages. "
                       "Use /help for more information.\n");
                break;
            }

            PayloadData data = {0};
            memcpy(data.message.display_name, DISPLAY_NAME, DISPLAY_NAME_LEN + 1);
            strcpy((void *)data.message.message_content, (void *)cmd.data.message);

            client_send(PayloadType_Message, &data);
            printf("%s: %s\n", DISPLAY_NAME, data.message.message_content);
            break;
        }

        case CommandType_Auth: {
            if (STATE != State_Start && STATE != State_NotAuth) {
                eprint("You have been authenticated. No need to do it again\n");
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
                eprint("Use /auth to authenticate first before joining a channel. "
                       "Use /help for more information.\n");
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
        error_clear();
        PayloadData data;
        strcpy((void *)data.err.display_name, (char *)DISPLAY_NAME);
        strcpy((void *)data.err.message_content, "Something went wrong when trying to send payload");
        CURRENT_PAYLOAD.payload = payload_new(PayloadType_Err, &data);
        CONNECTION.send(&CONNECTION, CURRENT_PAYLOAD.payload);
        STATE = State_Error;
    }
    
    /// If error still occur, then just terminate the programk
    if (get_error()) {
        STATE = State_End;
        return;
    }

    CURRENT_PAYLOAD.confirmed = CONNECTION.args.mode == Mode_TCP;
    CURRENT_PAYLOAD.timestamp = timestamp_now();
}
