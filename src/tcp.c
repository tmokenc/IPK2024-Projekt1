/**
 * @file tcp.c
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 03/03/2024
 * @brief Implementation for the tcp.h module
 */

#include "tcp.h"
#include "error.h"
#include "bytes.h"
#include "trie.h"
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>


/**
 * @brief Size of the buffer used for data transmission.
 *
 * This macro defines the size of the buffer used for data transmission.
 * A value of 4 KiB (4096 bytes) is considered reasonable for modern systems
 * and is sufficient to hold the longest packet in the IPK2024 protocol
 * due to limitations in its specification.
 */
#define BUFFER_SIZE (4 * 1024)

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
}

void tcp_destroy() {
    trie_free(TCP_TRIE);
}

void tcp_connect(Connection *conn) {
    // setup the trie
    tcp_setup();
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

    for (int i = 0; prefixes[i]; i++) {
        trie_insert(TCP_TRIE, (void *)prefixes[i], payload_type[i]);
    }

    struct sockaddr *address = conn->address_info->ai_addr;
    socklen_t address_len = conn->address_info->ai_addrlen;

    if (connect(conn->sockfd, address, address_len) != 0) {
        set_error(Error_Connection);
        fprintf(stderr, "tcp: Cannot connect to the server %s \n", conn->address_info->ai_canonname);
    }
}

void tcp_send(Connection *conn, Payload payload) {
    // to not allocate new Bytes each time this function is called
    static Bytes bytes;

    tcp_serialize(&payload, &bytes);
    if (get_error()) return;

    if (send(conn->sockfd, bytes.data, bytes.len, MSG_DONTWAIT) < 0) {
        set_error(Error_Connection);
        fprintf(stderr, "ERROR tcp: Cannot send packet to the server\n");
    }

    bytes_clear(&bytes);
}

Payload tcp_receive(Connection *conn) {
    uint8_t buffer[BUFFER_SIZE];
    Payload payload;

    ssize_t len = recv(conn->sockfd, buffer, BUFFER_SIZE, MSG_DONTWAIT);

    if (len < 0) {
        set_error(Error_Connection);
        fprintf(stderr, "ERROR tcp: Cannot receive packet from the server\n");
    } else {
        payload = tcp_deserialize(buffer, len);
    }

    return payload;
}

void tcp_disconnect(Connection *conn) {
    Payload payload;
    payload.type = PayloadType_Bye;
    tcp_send(conn, payload);
    shutdown(conn->sockfd, SHUT_RDWR);
    tcp_destroy();
}

int tcp_next_timeout(Connection *conn) {
    (void)conn;
    return -1;
}

void tcp_serialize(const Payload *payload, Bytes *buffer) {
    #define PUSH(str) \
        bytes_push_c_str(buffer, (char *)str); \
        if (get_error()) return

    switch (payload->type) {
        case PayloadType_Confirm:
            set_error(Error_InvalidPayload);
            return;

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
            PUSH("OK");
            PUSH(" IS ");
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

}

Payload tcp_deserialize(const uint8_t *bytes, size_t len) {
    Payload payload;
    ssize_t read;

    Bytes buffer = bytes_new();
    bytes_push_arr(&buffer, bytes, len);

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
            READ(join, channel_id); SKIP_STR(" AS ");
            READ(join, display_name); SKIP_STR("\r\n");
            break;

        case PayloadType_Auth:
            bytes_skip_first_n(&buffer, strlen("AUTH "));
            READ(auth, username); SKIP_STR(" AS ");
            READ(auth, display_name); SKIP_STR(" USING ");
            READ(auth, secret); SKIP_STR("\r\n");
            break;

        case PayloadType_Reply:
            bytes_skip_first_n(&buffer, strlen("REPLY "));
            bool result = true;
            if (bytes[0] == 'N') {
                result = false;
                bytes_skip_first_n(&buffer, 1);
            }

            SKIP_STR("OK IS ");
            READ(reply, message_content); SKIP_STR("\r\n");
            payload.data.reply.result = result;
            break;

        case PayloadType_Message:
            bytes_skip_first_n(&buffer, strlen("MSG FROM "));
            READ(message, display_name); SKIP_STR(" IS ");
            READ(message, message_content); SKIP_STR("\r\n");
            break;

        case PayloadType_Err:
            bytes_skip_first_n(&buffer, strlen("ERR FROM "));
            READ(err, display_name); SKIP_STR(" IS ");
            READ(err, message_content); SKIP_STR("\r\n");
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
    if (ch < 'A' || ch > 'Z') return -1;
    return ch - 'A';
}
