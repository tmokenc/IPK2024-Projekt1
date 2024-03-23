/**
 * @file udp.c
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 03/03/2024
 * @brief Implementation for udp.h
 */

#include "udp.h"
#include "connection.h"
#include "error.h"
#include "payload.h"
#include <sys/time.h>
#include <netdb.h>
#include <string.h>

typedef unsigned long long Timestamp;

struct current_payload {
    Bytes bytes;
    uint16_t id;
    int retry_count;
    Timestamp timestamp;
};

static void _send(Connection *conn, Bytes bytes);

/// Get the current timestamp in milli
static Timestamp current_timestamp_millis();

static size_t read_message_id(const Bytes *bytes, uint16_t *output);
static size_t read_result(const Bytes *bytes, uint8_t *output);

struct current_payload current_payload[1];

void udp_connect(Connection *conn) {
    // since it's UDP, it does not have to establish a connection to the server
    (void)conn;
}

void udp_send(Connection *conn, Payload payload) {
    current_payload->bytes = udp_serialize(&payload);
    current_payload->retry_count = 0;
    current_payload->id = payload.id;
    current_payload->timestamp = current_timestamp_millis();

    _send(conn, current_payload->bytes);

    conn->state = ConnectionState_WaitingAck;
}

Payload udp_receive(Connection *conn) {
    Payload payload = {0};

    Bytes buffer = bytes_new();

    struct sockaddr *address = conn->address_info->ai_addr;
    socklen_t address_len = conn->address_info->ai_addrlen;
    int flags = 0;
    ssize_t bytes_rx = recvfrom(conn->sockfd, buffer.data, BYTES_SIZE, flags, address, &address_len);

    if (bytes_rx < 0) {
        set_error(Error_Connection);
        perror("ERR: Cannot receive packet from server");
    } else {
        buffer.len = bytes_rx;
        payload = udp_deserialize(buffer);

        if (conn->state == ConnectionState_WaitingAck
            && payload.type == PayloadType_Confirm
            && payload.id == current_payload->id
        ) {
            conn->state = ConnectionState_Idle;
        }
    }

    return payload;
}

void udp_disconnect(Connection *conn) {
    // same as udp_connect
    (void)conn;
}

int udp_next_timeout(Connection *conn) {
    if (conn->state != ConnectionState_WaitingAck) {
        return -1;
    }

    int now = current_timestamp_millis();
    int elapsed = now - current_payload->timestamp;

    if (elapsed >= conn->args.udp_timeout) {
        if (current_payload->retry_count >= conn->args.udp_retransmissions) {
            fprintf(stderr, "ERR: Cannot send the message (timeout)\n");
            conn->state = ConnectionState_Idle;
            return -1;
        }

        _send(conn, current_payload->bytes);
        current_payload->timestamp = now;
        current_payload->retry_count += 1;
    }

    // do this once again since the previous operations may takes some time to complete.
    now = current_timestamp_millis();
    elapsed = now - current_payload->timestamp;

    return conn->args.udp_timeout - elapsed;
}

Bytes udp_serialize(const Payload *payload) {
    Bytes buffer = bytes_new();
    uint8_t header[3];

    header[0] = payload->type;
    header[1] = payload->id >> 8;
    header[2] = payload->id & 0xFF;

    bytes_push_arr(&buffer, header, 3);
    if (get_error()) return buffer;

    #define PUSH_BYTES(bytes) \
        bytes_push_c_str(&buffer, (void *)bytes); \
        bytes_push(&buffer, 0); \
        if (get_error()) return buffer

    switch (payload->type) {
        case PayloadType_Bye:
        case PayloadType_Confirm:
            break;

        case PayloadType_Reply:
            bytes_push(&buffer, payload->data.reply.result);
            bytes_push(&buffer, payload->data.reply.ref_message_id >> 8);
            bytes_push(&buffer, payload->data.reply.ref_message_id & 0xFF);
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

    return buffer;
}

Payload udp_deserialize(Bytes buffer) {
    Payload payload = {0};

    if (buffer.len < 3) {
        set_error(Error_InvalidPayload);
        return payload;
    }

    size_t read = 0;
    uint8_t type = bytes_get(&buffer)[0];

    if (type != PayloadType_Confirm
        && type != PayloadType_Reply
        && type != PayloadType_Auth
        && type != PayloadType_Join
        && type != PayloadType_Message
        && type != PayloadType_Err
        && type != PayloadType_Bye
    ) {
        set_error(Error_InvalidPayload);
        return payload;
    }

    payload.type = type;
    bytes_skip_first_n(&buffer, 1);

    read_message_id(&buffer, &payload.id);
    bytes_skip_first_n(&buffer, 2);

    #define READ_FUNC(func, output) \
        read = read_##func(&buffer, (void *)output); \
        if (read <= 0) { \
            set_error(Error_InvalidPayload); \
            return payload; \
        } \
        bytes_skip_first_n(&buffer, read)

    #define READ(select, buf) \
        read = read_##buf(payload.data.select.buf, &buffer); \
        if (read <= 0 || bytes_get(&buffer)[read] != 0) { \
            set_error(Error_InvalidPayload); \
            return payload; \
        } \
        bytes_skip_first_n(&buffer, read + 1);

    switch (type) {
        case PayloadType_Reply:
            READ_FUNC(result, &payload.data.reply.result);
            READ_FUNC(message_id, &payload.data.reply.ref_message_id);
            READ(reply, message_content);
            break;
        case PayloadType_Auth:
            READ(auth, username);
            READ(auth, display_name);
            READ(auth, secret);
            break;
        case PayloadType_Join:
            READ(join, channel_id);
            READ(join, display_name);
            break;
        case PayloadType_Message:
            READ(message, display_name);
            READ(message, message_content);
            break;
        case PayloadType_Err: READ(err, display_name);
            READ(err, message_content);
            break;
        case PayloadType_Confirm:
        case PayloadType_Bye:
            break;
    }

    if (buffer.len != 0) {
        set_error(Error_InvalidPayload);
    }

    return payload;
}

static size_t read_message_id(const Bytes *bytes, uint16_t *output) {
    if (bytes->len < 2) return -1;
    const uint8_t *slice = bytes_get(bytes);
    *output = (slice[0] << 8) | slice[1];
    return 2;
}

static size_t read_result(const Bytes *bytes, uint8_t *output) {
    if (!bytes->len) {
        set_error(Error_InvalidPayload);
        return 0;
    }

    uint8_t res = bytes_get(bytes)[0];

    if (res != 0 && res != 1) {
        set_error(Error_InvalidPayload);
        return 0;
    }

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
        perror("ERR: Cannot send packet to the server");
    }
}

static Timestamp current_timestamp_millis() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (Timestamp)(tv.tv_sec) * 1000 + (Timestamp)(tv.tv_usec) / 1000;
}
