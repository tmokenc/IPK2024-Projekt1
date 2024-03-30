/**
 * @file tcp.c
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 03/03/2024 @brief Implementation for the tcp.h module
 */

#include "tcp.h"
#include "error.h"
#include "bytes.h"
#include "trie.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <poll.h>

/**
 * Timeout when waiting for the connection to be established (5 seconds).
 */
#define TCP_CONNECT_TIMEOUT 5000

/**
 * @brief Check if a byte array starts with a given null-terminated C string.
 *
 * @param bytes Pointer to the byte array to check.
 * @param len Length of the byte array.
 * @param str Pointer to the null-terminated C string to compare.
 * @return true if the byte array starts with the given C string, false otherwise.
 */
static bool starts_with(const Bytes *bytes, const char *str);

static int tcp_prefix_index(uint8_t ch);

Trie *TCP_TRIE;

void tcp_setup() {
    TCP_TRIE = trie_new(tcp_prefix_index);
    if (get_error()) return;

    char *prefixes[] = {"JOIN ", "AUTH ", "MSG FROM ", "ERR FROM ", "REPLY ", "BYE\r\n", NULL };
    PayloadType payload_type[] = {
        PayloadType_Join,
        PayloadType_Auth,
        PayloadType_Message,
        PayloadType_Err,
        PayloadType_Reply,
        PayloadType_Bye,
    };

    log("Establish TCP connection");
    for (int i = 0; prefixes[i]; i++) {
        trie_insert(TCP_TRIE, (void *)prefixes[i], payload_type[i]);
        if (get_error()) return;
    }

    log("Established");
}

void tcp_destroy() {
    trie_free(TCP_TRIE);
}

void tcp_connect(Connection *conn) {
    // setup the trie
    tcp_setup();
    if (get_error()) return;

    struct sockaddr *address = conn->address_info->ai_addr;
    socklen_t address_len = conn->address_info->ai_addrlen;

    int res = connect(conn->sockfd, address, address_len);

    if (res != 0 && errno != EINPROGRESS) {
        perror("ERR: Cannot connect to the server");
        set_error(Error_Connection);
        return;
    }

    struct pollfd fds[1];
    fds[0].fd = conn->sockfd;
    fds[0].events = POLLOUT;

    // Poll the socket infinitely to wait for the connection to be established
    int poll_result = poll(fds, 1, TCP_CONNECT_TIMEOUT);

    if (poll_result < 0) {
        perror("ERR: poll");
        set_error(Error_Socket);
    } else if (poll_result == 0) {
        fprintf(stderr, "ERR: Cannot establish connection to the server within %dms", TCP_CONNECT_TIMEOUT);
        set_error(Error_Socket);
    } else if (fds[0].revents & POLLOUT) {
        // printf("Connected to the server\n");
    }
}

void tcp_send(Connection *conn, Payload payload) {
    if (payload.type == PayloadType_Confirm) {
        /// We do not send confirm in TCP
        return;
    }
    
    Bytes bytes = tcp_serialize(&payload);

    if (get_error()) return;

    if (send(conn->sockfd, bytes.data, bytes.len, 0) < 0) {
        set_error(Error_Connection);
        perror("ERR: Cannot send packet to the server");
    }

    bytes_clear(&bytes);
}

Payload tcp_receive(Connection *conn) {
    log("Receiving TCP packet");
    Payload payload;
    Bytes buffer = bytes_new();

    ssize_t len = recv(conn->sockfd, buffer.data, BYTES_SIZE, 0);

    if (len < 0) {
        set_error(Error_Connection);
        perror("ERR: Cannot receive packet from the server");
    } else {
        buffer.len = len;
        payload = tcp_deserialize(buffer);
        logfmt("Received payload type %u", payload.type);
    }

    return payload;
}

void tcp_disconnect(Connection *conn) {
    log("Disconnecting TCP connection");
    shutdown(conn->sockfd, SHUT_RDWR);
    tcp_destroy();
}

Bytes tcp_serialize(const Payload *payload) {
    // Don't have to validate the input since it has been validated in command parsing state
    Bytes buffer = bytes_new();
    
    #define PUSH(str) \
        if (strlen((void *)str) == 0) { \
            set_error(Error_InvalidPayload); \
            return buffer; \
        } \
        bytes_push_c_str(&buffer, (char *)str); \
        if (get_error()) return buffer

    switch (payload->type) {
        case PayloadType_Confirm:
            set_error(Error_InvalidPayload);
            return buffer;

        case PayloadType_Auth:
            PUSH("AUTH ");
            PUSH(payload->data.auth.username);
            PUSH(" AS ");
            PUSH(payload->data.auth.display_name);
            PUSH(" USING ");
            PUSH(payload->data.auth.secret);
            break;

        case PayloadType_Reply:
            PUSH("REPLY ");
            if (!payload->data.reply.result) {
                PUSH("N");
            }
            PUSH("OK IS ");
            PUSH(payload->data.reply.message_content);
            break;

        case PayloadType_Join:
            PUSH("JOIN ");
            PUSH(payload->data.join.channel_id);
            PUSH(" AS ");
            PUSH(payload->data.join.display_name);
            break;

        case PayloadType_Message:
            PUSH("MSG FROM ");
            PUSH(payload->data.message.display_name);
            PUSH(" IS ");
            PUSH(payload->data.message.message_content);
            break;

        case PayloadType_Err:
            PUSH("ERR FROM ");
            PUSH(payload->data.message.display_name);
            PUSH(" IS ");
            PUSH(payload->data.message.message_content);
            break;

        case PayloadType_Bye:
            PUSH("BYE");
            break;
    }

    PUSH("\r\n");

    return buffer;
}

Payload tcp_deserialize(Bytes buffer) {
    Payload payload;
    ssize_t read;

    #define READ(select, buf) \
        read = read_##buf(payload.data.select.buf, &buffer); \
        if (read <= 0) { \
            set_error(Error_InvalidPayload); \
            return payload; \
        } \
        bytes_skip_first_n(&buffer, read)

    #define SKIP_STR(str) \
        if (!starts_with(&buffer, str)) { \
            set_error(Error_InvalidPayload); \
            return payload; \
        } \
        bytes_skip_first_n(&buffer, strlen(str))

    int maybe_payload_type = trie_match_prefix(TCP_TRIE, bytes_get(&buffer));

    switch (maybe_payload_type) {
        case PayloadType_Join:
            bytes_skip_first_n(&buffer, strlen("JOIN "));
            READ(join, channel_id); 
            SKIP_STR(" AS ");
            READ(join, display_name); 
            SKIP_STR("\r\n");
            break;

        case PayloadType_Auth:
            bytes_skip_first_n(&buffer, strlen("AUTH "));
            READ(auth, username); 
            SKIP_STR(" AS ");
            READ(auth, display_name); 
            SKIP_STR(" USING ");
            READ(auth, secret); 
            SKIP_STR("\r\n");
            break;

        case PayloadType_Reply:
            bytes_skip_first_n(&buffer, strlen("REPLY "));

            payload.data.reply.result = bytes_get(&buffer)[0] != 'N';
            bytes_skip_first_n(&buffer, !payload.data.reply.result);

            SKIP_STR("OK IS ");
            READ(reply, message_content); 
            SKIP_STR("\r\n");
            break;

        case PayloadType_Message:
            bytes_skip_first_n(&buffer, strlen("MSG FROM "));
            READ(message, display_name); 
            SKIP_STR(" IS ");
            READ(message, message_content); 
            SKIP_STR("\r\n");
            break;

        case PayloadType_Err:
            bytes_skip_first_n(&buffer, strlen("ERR FROM "));
            READ(err, display_name); 
            SKIP_STR(" IS ");
            READ(err, message_content); 
            SKIP_STR("\r\n");
            break;

        case PayloadType_Bye:
            bytes_skip_first_n(&buffer, strlen("BYE\r\n"));
            break;

        default:
            set_error(Error_InvalidPayload);
            return payload;
    }

    /// It should be fully deserialized with nothing left
    if (buffer.len != 0) set_error(Error_InvalidPayload);

    payload.type = maybe_payload_type;
    return payload;
}


static bool starts_with(const Bytes *bytes, const char *str) {
    if (bytes->len < strlen(str)) return false;
    return memcmp(bytes_get(bytes), str, strlen(str)) == 0;
}

static int tcp_prefix_index(uint8_t ch) {
    if (ch == ' ') return 26;
    ch |= 1 << 5; // an ASCII trick to make a character lowercase.
    if (ch >= 'a' && ch <= 'z') return ch - 'a';
    return -1;
}
