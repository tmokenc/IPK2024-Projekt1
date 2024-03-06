/**
 * @file tcp.c
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 03/03/2024
 * @brief Implementation for the tcp.h module
 */

#include "tcp.h"
#include "error.h"
#include "bytes.h"
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
static bool starts_with(const uint8_t *bytes, size_t len, const char *str);

/**
 * @brief Copy bytes from a byte array until it meets the specified null-terminated C string.
 *
 * This function copies bytes from the byte array to the output buffer until it encounters
 * the specified null-terminated C string (excluding the string itself). It also appended 
 * the NULL terminator to the output so it can act as a C string.
 *
 * @param bytes Pointer to the byte array to copy from.
 * @param bytes_len Length of the byte array.
 * @param ends_with Pointer to the null-terminated C string indicating the end condition.
 * @param output Pointer to the output buffer where bytes will be copied.
 * @param output_len Length of the output buffer.
 * @return Number of bytes read from the byte array (including the end condition)
 *         -1 if it cannot find the end condition within the output_len
 */
static ssize_t strcpy_until(
    const uint8_t *bytes, 
    size_t bytes_len, 
    const char *ends_with, 
    uint8_t *output, 
    size_t output_len
);



void tcp_connect(Connection *conn) {
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
}

int tcp_next_timeout(Connection *conn) {
    (void)conn;
    return -1;
}

void tcp_serialize(Payload *payload, Bytes *buffer) {
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
            PUSH("ERROR FROM ");
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

Payload tcp_deserialize(uint8_t *bytes, size_t len) {
    Payload payload;
    ssize_t read;

    #define EXPECT_NEXT(buf, buf_len, ends_with) \
        read = strcpy_until(bytes, len, ends_with, payload.data.buf, buf_len + 1); \
        if (read <= 0) { \
            set_error(Error_InvalidPayload); \
            return payload; \
        } \
        bytes += read; \
        len -= read

    if (starts_with(bytes, len, "JOIN ")) {
        bytes += strlen("JOIN ");
        len -= strlen("JOIN ");
        EXPECT_NEXT(join.channel_id, CHANNEL_ID_LEN, " AS ");
        EXPECT_NEXT(join.display_name, DISPLAY_NAME_LEN, "\r\n");
        payload.type = PayloadType_Join;
    }

    if (starts_with(bytes, len, "AUTH ")) {
        bytes += strlen("AUTH ");
        len -= strlen("AUTH ");
        EXPECT_NEXT(auth.username, USERNAME_LEN, " AS ");
        EXPECT_NEXT(auth.display_name, DISPLAY_NAME_LEN, " USING ");
        EXPECT_NEXT(auth.secret, SECRET_LEN, "\r\n");
        payload.type = PayloadType_Auth;
    }

    if (starts_with(bytes, len, "MSG FROM ")) {
        bytes += strlen("MSG FROM ");
        len -= strlen("MSG FROM ");
        EXPECT_NEXT(message.display_name, DISPLAY_NAME_LEN, " IS ");
        EXPECT_NEXT(message.message_content, MESSAGE_CONTENT_LEN, "\r\n");
        payload.type = PayloadType_Message;
    }

    if (starts_with(bytes, len, "ERROR FROM ")) {
        bytes += strlen("ERROR FROM ");
        len -= strlen("ERROR FROM ");
        EXPECT_NEXT(err.display_name, DISPLAY_NAME_LEN, " IS ");
        EXPECT_NEXT(err.message_content, MESSAGE_CONTENT_LEN, "\r\n");
        payload.type = PayloadType_Err;
    }

    if (starts_with(bytes, len, "ERROR FROM ")) {
        bytes += strlen("ERROR FROM ");
        len -= strlen("ERROR FROM ");
        EXPECT_NEXT(err.message_content, MESSAGE_CONTENT_LEN, "\r\n");
        payload.type = PayloadType_Reply;
    }

    if (starts_with(bytes, len, "BYE\r\n")) {
        bytes += strlen("BYE\r\n");
        len -= strlen("BYE\r\n");
        payload.type = PayloadType_Bye;
    }

    /// It should be fully deserialized with nothing left
    if (len != 0) set_error(Error_InvalidPayload);

    return payload;
}


static bool starts_with(const uint8_t *bytes, size_t len, const char *str) {
    if (len < strlen(str)) return false;
    return memcmp((char *)bytes, str, strlen(str)) == 0;
}

static ssize_t strcpy_until(
    const uint8_t *bytes,
    size_t bytes_len,
    const char *ends_with,
    uint8_t *output,
    size_t output_len
) {
    size_t len = strlen(ends_with);
    ssize_t index = 0;

    while(bytes_len - index >= len && output_len - index >= 1) {
        if (starts_with(bytes + index, bytes_len - index, ends_with)) {
            output[index + 1] = 0;
            return index + len;
        }

        output[index] = bytes[index];

        index++;
    }

    return -1;
}
