/**
 * @file udp.c
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 03/03/2024
 * @brief Implementation for udp.h
 */

#include "udp.h"
#include "error.h"
#include <sys/time.h>
#include "../lib/pqueue.h"
#include <netdb.h>

#define QUEUE_SIZE 10

static void _send(Connection *conn, Bytes bytes);

/**
 * Define function needed to use with priority queue
 *
 */
static int cmp_pri(pqueue_pri_t next, pqueue_pri_t curr);
static pqueue_pri_t get_pri(void *a);
static void set_pri(void *a, pqueue_pri_t pri);
static size_t get_pos(void *a);
static void set_pos(void *a, size_t pos);

/// Get the current timestamp in milli
static pqueue_pri_t current_timestamp_millis();

/**
 * @brief Count the number of bytes up to the first 0 (including that 0).
 *
 * @param bytes Pointer to the byte array.
 * @return The number of bytes up to the first 0 (including that 0).
 * @note this function assumes that the byte will be valid and always contains the ending byte (byte 0)
 */
static size_t byte_count(uint8_t *bytes);

static size_t read_message_id(uint8_t *bytes, size_t len, uint16_t *output);
static size_t read_id(uint8_t *bytes, size_t len, uint8_t *output, size_t limit);
static size_t read_display_name(uint8_t *bytes, size_t len, uint8_t *output);
static size_t read_message_content(uint8_t *bytes, size_t len, uint8_t *output);
static size_t read_result(uint8_t *bytes, size_t len, uint8_t *output);


typedef struct {
    pqueue_pri_t timestamp;
    int retry_count;
    Bytes bytes;
    size_t position;
} QueueNode;


pqueue_t *QUEUE;

void udp_connect(Connection *conn) {
    // since it's UDP, it does not have to establish a connection to the server
    (void)conn;

    // Setup the queue for connection
    QUEUE = pqueue_init(QUEUE_SIZE, cmp_pri, get_pri, set_pri, get_pos, set_pos);

    if (!QUEUE) {
        set_error(Error_OutOfMemory);
    }
}

void udp_send(Connection *conn, Payload payload) {
    (void)conn;
    (void)payload;
    // TODO

}

Payload udp_receive(Connection *conn) {
    (void)conn;
    Payload payload = {0};
    // TODO
    return payload;
}

void udp_disconnect(Connection *conn) {
    (void)conn;
    Payload payload;
    payload.type = PayloadType_Bye;
    udp_send(conn, payload);
}

int udp_next_timeout(Connection *conn) {
    (void)conn;

    QueueNode *peek;

    int now = current_timestamp_millis();

    while ((peek = pqueue_peek(QUEUE))) {
        int elapsed = now - peek->timestamp;

        if (elapsed < conn->args.udp_timeout) {
            break;
        }

        // Timeout

        if (peek->retry_count >= conn->args.udp_retransmissions) {
            pqueue_pop(QUEUE);
            continue;
        }

        // Retry
        _send(conn, peek->bytes);
        peek->retry_count += 1;
        pqueue_change_priority(QUEUE, current_timestamp_millis(), peek);
    }

    peek = pqueue_peek(QUEUE);

    if (!peek) {
        return -1;
    }

    // since the loop above could take sometime and it make the measurement incorrect
    int elapsed = current_timestamp_millis() - peek->timestamp;
    // 0 as timeout means it should not wait for the timeout and get the poll result immediately
    if (elapsed < 0) elapsed = 0;

    return peek == NULL ? -1 : conn->args.udp_timeout - elapsed;
}

static int cmp_pri(pqueue_pri_t next, pqueue_pri_t curr) {
    return next < curr;
}

static pqueue_pri_t get_pri(void *a) {
    return ((QueueNode *)a)->timestamp;
}

static void set_pri(void *a, pqueue_pri_t pri) {
    ((QueueNode *)a)->timestamp = pri;
}

static size_t get_pos(void *a) {
    return ((QueueNode *)a)->position;
}

static void set_pos(void *a, size_t pos) {
    ((QueueNode *)a)->position = pos;
}

void udp_serialize(Payload *payload, Bytes *buffer) {
    uint8_t header[3];

    header[0] = payload->type;
    header[1] = payload->id >> 8;
    header[2] = payload->id & 0xFF;

    bytes_push_arr(buffer, header, 3);
    if (get_error()) return;

    #define PUSH_BYTES(bytes) \
        bytes_push_arr(buffer, bytes, byte_count(bytes)); \
        if (get_error()) return

    switch (payload->type) {
        case PayloadType_Bye:
        case PayloadType_Confirm:
            break;

        case PayloadType_Reply:
            bytes_push(buffer, payload->data.reply.result);
            bytes_push(buffer, payload->data.reply.ref_message_id >> 8);
            bytes_push(buffer, payload->data.reply.ref_message_id & 0xFF);
            PUSH_BYTES((uint8_t *)payload->data.reply.message_content);
            break;

        case PayloadType_Auth:
            PUSH_BYTES((uint8_t *)payload->data.auth.username);
            PUSH_BYTES((uint8_t *)payload->data.auth.display_name);
            PUSH_BYTES((uint8_t *)payload->data.auth.secret);
            break;

        case PayloadType_Join:
            PUSH_BYTES((uint8_t *)payload->data.join.channel_id);
            PUSH_BYTES((uint8_t *)payload->data.join.display_name);
            break;

        case PayloadType_Message:
            PUSH_BYTES((uint8_t *)payload->data.message.display_name);
            PUSH_BYTES((uint8_t *)payload->data.message.message_content);
            break;

        case PayloadType_Err:
            PUSH_BYTES((uint8_t *)payload->data.message.display_name);
            PUSH_BYTES((uint8_t *)payload->data.message.message_content);
            break;
    }

}

Payload udp_deserialize(uint8_t *bytes, size_t len) {
    Payload payload = {0};

    if (len < 3) {
        set_error(Error_InvalidPayload);
        return payload;
    }

    size_t read;

    uint8_t type = bytes[0];
    len++;

    if (type != PayloadType_Confirm
        || type != PayloadType_Reply
        || type != PayloadType_Auth
        || type != PayloadType_Join
        || type != PayloadType_Message
        || type != PayloadType_Err
        || type != PayloadType_Bye
    ) {
        set_error(Error_InvalidPayload);
        return payload;
    }

    payload.type = type;
    read_message_id(bytes, len, &payload.id);

    #define READ_ID(limit, output) \
        read = read_id(bytes, len, output, limit); \
        if (read <= 0) { \
            set_error(Error_InvalidPayload); \
            return payload; \
        } \
        bytes += read; \
        len -= read

    #define READ(func, output) \
        read = read_##func(bytes, len, output); \
        if (read <= 0) { \
            set_error(Error_InvalidPayload); \
            return payload; \
        } \
        bytes += read; \
        len -= read

    switch (type) {
        case PayloadType_Reply:
            READ(result, (uint8_t *)payload.data.reply.result);
            READ(message_id, &payload.data.reply.ref_message_id);
            READ(message_content, payload.data.reply.message_content);
            break;
        case PayloadType_Auth:
            READ_ID(USERNAME_LEN, payload.data.auth.username);
            READ(display_name, payload.data.auth.display_name);
            READ_ID(SECRET_LEN, payload.data.auth.secret);
            break;
        case PayloadType_Join:
            READ_ID(CHANNEL_ID_LEN, payload.data.join.channel_id);
            READ(display_name, payload.data.join.display_name);
            break;
        case PayloadType_Message:
            READ(display_name, payload.data.message.display_name);
            READ(message_content, payload.data.message.message_content);
            break;
        case PayloadType_Err:
            READ(display_name, payload.data.message.display_name);
            READ(message_content, payload.data.message.message_content);
            break;
        case PayloadType_Confirm:
        case PayloadType_Bye:
            break;
    }

    if (len != 0) {
        set_error(Error_InvalidPayload);
    }

    return payload;
}

static size_t byte_count(uint8_t *bytes) {
    size_t i = 0;
    while (bytes[i++]) {}
    return i;
}

static size_t read_message_id(uint8_t *bytes, size_t len, uint16_t *output) {
    if (len < 2) return -1;
    *output = (bytes[0] << 8) | bytes[1 + 1];
    return 2;
}

static size_t read_id(uint8_t *bytes, size_t len, uint8_t *output, size_t limit) {
    for (size_t i = 0; i < len && i < limit; i++) {
        char ch = bytes[i];

        if ((ch < 'a' || ch > 'z')      // is lower case
            && (ch < 'A' || ch > 'Z')   // is upper case
            && (ch < '0' || ch > '9')   // is number
            && ch != '-'                // dash
        ) {
            break;
        }

        output[i] = ch;

        // Found the end
        // i is the index (starts from 0), that's why it need + 1 to get the total number of bytes
        if (ch == 0) return i + 1;
    }

    return 0;
}

static size_t read_display_name(uint8_t *bytes, size_t len, uint8_t *output) {
    for (size_t i = 0; i < len && i < DISPLAY_NAME_LEN + 1; i++) {
        char ch = bytes[i];
        if (ch < 0x20 || ch > 0x7E) break;
        output[i] = ch;

        if (ch == 0) return i + 1;
    }

    return 0;
}

static size_t read_message_content(uint8_t *bytes, size_t len, uint8_t *output) {
    for (size_t i = 0; i < len && i < MESSAGE_CONTENT_LEN + 1; i++) {
        char ch = bytes[i];
        if (ch < 0x21 || ch > 0x7E) break;
        output[i] = ch;

        // done
        if (ch == 0) return i + 1;
    }

    return 0;
}

static size_t read_result(uint8_t *bytes, size_t len, uint8_t *output) {
    if (!len) {
        set_error(Error_InvalidPayload);
        return 0;
    }

    uint8_t res = bytes[0];

    if (res != 0 || res != 1) {
        set_error(Error_InvalidPayload);
        return 0;
    }

    // reply is the only payload that has the `result` in its data field
    *output = res;
    return 1;
}

static void _send(Connection *conn, Bytes bytes) {
    struct sockaddr *address = conn->address_info->ai_addr;
    socklen_t address_len = conn->address_info->ai_addrlen;
    int flags = 0;
    ssize_t bytes_tx = sendto(conn->sockfd, bytes.data, bytes.len, flags, address, address_len);

    if (bytes_tx != (ssize_t)bytes.len) {
        set_error(Error_Connection);
    }
}

static pqueue_pri_t current_timestamp_millis() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000;
}
