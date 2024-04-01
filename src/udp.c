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
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>

static size_t read_message_id(const Bytes *bytes, uint16_t *output);
static size_t read_result(const Bytes *bytes, uint8_t *output);

void udp_connect(Connection *conn) {
    // UDP is connectionless protocol, it does not have to establish a connection to the server
    (void)conn;
    log("Connected");
}

void udp_send(Connection *conn, Payload payload) {
    logfmt("Sending payload type %u", payload.type);
    Bytes bytes = udp_serialize(&payload);

    #ifdef DEBUG_F
    const uint8_t *bytes_slice = bytes_get(&bytes);
    for (size_t i = 0; i < bytes.len; i++) {
        fprintf(stderr, "0x%02x ", bytes_slice[i]);
    }
    fprintf(stderr, "\n");
    #endif


    struct sockaddr *address = conn->address_info->ai_addr;
    socklen_t address_len = conn->address_info->ai_addrlen;
    int flags = 0;

    ssize_t bytes_tx = sendto(conn->sockfd, bytes.data, bytes.len, flags, address, address_len);

    if (bytes_tx != (ssize_t)bytes.len) {
        set_error(Error_Connection);
        perror("ERR: Cannot send packet to the server");
    }
}

Payload udp_receive(Connection *conn) {
    log("Received payload");

    Payload payload = {0};
    Bytes buffer = bytes_new();

    struct sockaddr_in address;
    socklen_t address_len = sizeof(address);

    int flags = 0;
    ssize_t bytes_rx = recvfrom(conn->sockfd, buffer.data, BYTES_SIZE, flags, (struct sockaddr *)&address, &address_len);

    if (bytes_rx < 0) {
        set_error(Error_Connection);
        perror("ERR: Cannot receive packet from server");
        return payload;
    }

    // Address of the server
    struct sockaddr_in *server_address = (struct sockaddr_in *)conn->address_info->ai_addr;

    // Check if the incoming packet is from the same address
    if (memcmp(&address.sin_addr, &server_address->sin_addr, sizeof(address.sin_addr)) != 0) {
        set_error(Error_RecvFromWrongAddress);
        return payload;
    }

    // Change port based on the sender port
    logfmt("Port %u", ntohs(address.sin_port));
    server_address->sin_port = address.sin_port;

    buffer.len = bytes_rx;
    payload = udp_deserialize(buffer);
    
    logfmt("Received payload with ID %u", payload.id);
    return payload;
}

void udp_disconnect(Connection *conn) {
    log("Disconnected");
    // UDP is connectionless, it does not require any disconnection process.
    (void)conn;
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
        logfmt("Pushing %s", bytes); \
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
            PUSH_BYTES(payload->data.reply.message_content);
            break;

        case PayloadType_Auth:
            PUSH_BYTES(payload->data.auth.username);
            PUSH_BYTES(payload->data.auth.display_name);
            PUSH_BYTES(payload->data.auth.secret);
            break;

        case PayloadType_Join:
            PUSH_BYTES(payload->data.join.channel_id);
            PUSH_BYTES(payload->data.join.display_name);
            break;

        case PayloadType_Message:
            PUSH_BYTES(payload->data.message.display_name);
            PUSH_BYTES(payload->data.message.message_content);
            break;

        case PayloadType_Err:
            PUSH_BYTES(payload->data.err.display_name);
            PUSH_BYTES(payload->data.err.message_content);
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

    payload.type = type;
    bytes_skip_first_n(&buffer, 1);

    read_message_id(&buffer, &payload.id);
    bytes_skip_first_n(&buffer, 2);

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
        case PayloadType_Err: 
            READ(err, display_name);
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

    logfmt("Reading result. Got %d", res);
    *output = res;
    return 1;
}
