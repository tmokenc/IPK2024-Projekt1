/**
 * @file payload.c
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 02/03/2024
 * @brief Implementation of the payload.h
 */

#include "payload.h"
#include "error.h"
#include <string.h>

enum DataType {
    DataType_None,
    DataType_PayloadType,
    DataType_Result,
    DataType_MessageID,
    DataType_ChannelID,
    DataType_MessageContent,
    DataType_Username,
    DataType_Secret,
    DataType_DisplayName,
};

static void serialize_tcp(Payload *payload, Bytes *buffer);
static void serialize_udp(Payload *payload, Bytes *buffer);
static void deserialize_tcp(const uint8_t *bytes, size_t size, PayloadDeserialization *result);
static void deserialize_udp(const uint8_t *bytes, size_t size, PayloadDeserialization *result);

/**
 * @brief Count the number of bytes up to the first 0 (including that 0).
 *
 * @param bytes Pointer to the byte array.
 * @return The number of bytes up to the first 0 (including that 0).
 * @note this function assumes that the byte will be valid and always contains the ending byte (byte 0)
 */
static size_t byte_count(uint8_t *bytes);

/**
 * @brief Helper functions for deserializing specific data types from a UDP byte stream.
 *
 * This group of functions includes helper functions to deserialize various data types such as payload type,
 * result, message ID, channel ID, message content, username, and display name from a UDP byte stream into
 * a PayloadDeserialization structure.
 */
static size_t deserialize_udp_payload_type(const uint8_t *bytes, size_t size, PayloadDeserialization *result);
static size_t deserialize_udp_result(const uint8_t *bytes, size_t size, PayloadDeserialization *result);
static size_t deserialize_udp_message_id(const uint8_t *bytes, size_t size, PayloadDeserialization *result);
static size_t deserialize_udp_channel_id(const uint8_t *bytes, size_t size, PayloadDeserialization *result);
static size_t deserialize_udp_message_content(const uint8_t *bytes, size_t size, PayloadDeserialization *result);
static size_t deserialize_udp_username(const uint8_t *bytes, size_t size, PayloadDeserialization *result);
static size_t deserialize_udp_display_name(const uint8_t *bytes, size_t size, PayloadDeserialization *result);
static size_t deserialize_udp_secret(const uint8_t *bytes, size_t size, PayloadDeserialization *result);

/**
 * Helper Functions
 * use a 32-bit unsigned integer to store the state of the program
 * states are encoded in base 16 which can be represented in 4 bits
 * thus allows it stores up to 8 states, more than enough for the current IPK2024 protocol.
 * The states are stored in queue (first in first out)
 */

/**
 * @brief Pushes a new deserialization state based on the data type into the state queue.
 * @param current_state Pointer to the current state of the program.
 * @param type The data type to encode in the state.
 */
static void deserialization_state_push(uint32_t *current_state, enum DataType type);

/**
 * @brief Gets the next data type from the deserialization state queue.
 * @param current_state The current state of the program.
 * @return The next data type from the state queue.
 */
static enum DataType deserialization_state_get(uint32_t current_state);

/**
 * @brief Pops the oldest deserialization state from the state queue.
 * @param current_state Pointer to the current state of the program.
 */
static void deserialization_state_pop(uint32_t *current_state);

/**
 * @brief Scans text data from a byte buffer and copies it into a character buffer.
 * @param bytes Pointer to the byte buffer containing the text data.
 * @param len Length of the byte buffer.
 * @param buf Pointer to the character buffer to store the scanned text.
 * @param buf_len Length of the character buffer.
 * @return The number of bytes scanned and copied into the character buffer.
 *         -1 if it cannot read 
 */
static size_t scan_text(const uint8_t *bytes, size_t len, char *buf, size_t buf_len);

/// The ID of the next payload, this will be incremented each time the payload_new function is called
MessageID NEXT_MESSAGE_ID;

Payload payload_new(PayloadType type, PayloadData *data) {
    Payload payload;
    payload.type = type;
    payload.id = NEXT_MESSAGE_ID++;
    if (data) payload.data = *data;

    return payload;
}

Bytes payload_serialize(Payload *payload, Mode mode) {
    Bytes result = bytes_new();

    switch (mode) {
        case Mode_TCP:
            serialize_tcp(payload, &result);
            break;
        case Mode_UDP:
            serialize_udp(payload, &result);
            break;
    }

    return result;
}

void payload_deserialize(const uint8_t *bytes, size_t len, Mode mode, PayloadDeserialization *result) {
    switch (mode) {
        case Mode_TCP:
            deserialize_tcp(bytes, len, result);
            break;
        case Mode_UDP:
            deserialize_udp(bytes, len, result);
            break;
    }
}

static void serialize_tcp(Payload *payload, Bytes *buffer) {
    #define PUSH(str) \
        bytes_push_c_str(buffer, str); \
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
            // TODO OK or NOK
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

static void serialize_udp(Payload *payload, Bytes *buffer) {
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

static void deserialize_tcp(const uint8_t *bytes, size_t len, PayloadDeserialization *result) {

    // TODO
}

static void deserialize_udp(const uint8_t *bytes, size_t len, PayloadDeserialization *result) {
    if (!len) return;

    // New payload
    if (!result->current_progress) {
        deserialization_state_push(&result->current_progress, DataType_PayloadType);
        deserialization_state_push(&result->current_progress, DataType_MessageID);
    }

    bytes_push_arr(&result->leftover, bytes, len);

    if (get_error()) return;

    const uint8_t *arr = result->leftover.data;
    size_t bytes_left = result->leftover.len;
    size_t cursor = 0;
    bool done = false;

    #define CASE(type, func) case DataType_##type: \
        processed = deserialize_udp_##func(arr + cursor, bytes_left, result); \
        break

    while (!done || !bytes_left) {
        size_t processed = 0;
        switch (deserialization_state_get(result->current_progress)) {
            CASE(PayloadType, payload_type);
            CASE(Result, result);
            CASE(MessageID, message_id);
            CASE(ChannelID, channel_id);
            CASE(MessageContent, message_content);
            CASE(Username, username);
            CASE(Secret, secret);
            CASE(DisplayName, display_name);
            case DataType_None: 
                done = true; 
                break;
        }

        if (get_error()) return;

        cursor += processed;
        bytes_left -= processed;

        if (processed) {
            deserialization_state_pop(&result->current_progress);
        } else {
            done = true;
        }
    }

    bytes_remove_first_n(&result->leftover, cursor);
}

static size_t scan_text(const uint8_t *bytes, size_t len, char *buf, size_t buf_len) {
    for (size_t i = 0; i < len && i < buf_len; i++) {
        char ch = bytes[i];

        if ((ch < 'a' || ch > 'z')      // is lower case
            && (ch < 'A' || ch > 'Z')   // is upper case
            && (ch < '0' || ch > '9')   // is number
            && ch != '-'                // dash
        ) {
            break;
        }

        buf[i] = ch;

        // Found the end
        // i is the index (starts from 0), that's why it need + 1 to get the total number of bytes
        if (ch == 0) return i + 1; 
    }

    return -1;
}

static size_t deserialize_udp_payload_type(const uint8_t *bytes, size_t size, PayloadDeserialization *result) {
    if (!size) {
        set_error(Error_InvalidPayload);
        return 0;
    }

    uint8_t type = bytes[0];

    if (type != PayloadType_Confirm
        || type != PayloadType_Reply
        || type != PayloadType_Auth
        || type != PayloadType_Join
        || type != PayloadType_Message
        || type != PayloadType_Err
        || type != PayloadType_Bye
    ) {
        set_error(Error_InvalidPayload);
        return 0;
    }

    result->payload.type = type;

    switch (result->payload.type) {
        case PayloadType_Reply:
            deserialization_state_push(&result->current_progress, DataType_Result);
            deserialization_state_push(&result->current_progress, DataType_MessageID);
            deserialization_state_push(&result->current_progress, DataType_MessageContent);
            break;
        case PayloadType_Auth:
            deserialization_state_push(&result->current_progress, DataType_Username);
            deserialization_state_push(&result->current_progress, DataType_DisplayName);
            deserialization_state_push(&result->current_progress, DataType_Secret);
            break;
        case PayloadType_Join:
            deserialization_state_push(&result->current_progress, DataType_ChannelID);
            deserialization_state_push(&result->current_progress, DataType_DisplayName);
            break;
        case PayloadType_Message:
        case PayloadType_Err:
            deserialization_state_push(&result->current_progress, DataType_DisplayName);
            deserialization_state_push(&result->current_progress, DataType_MessageContent);
            break;
        case PayloadType_Confirm:
        case PayloadType_Bye:
            break;
    }

    return 1;
}

static size_t deserialize_udp_result(const uint8_t *bytes, size_t size, PayloadDeserialization *result) {
    if (!size) {
        set_error(Error_InvalidPayload);
        return 0;
    }

    uint8_t res = bytes[0];

    if (res != 0 || res != 1) {
        set_error(Error_InvalidPayload);
        return 0;
    }

    // reply is the only payload that has the `result` in its data field
    result->payload.data.reply.result = res;
    return 1;
}

static size_t deserialize_udp_message_id(const uint8_t *bytes, size_t size, PayloadDeserialization *result) {
    if (size < 2) return 0;
    MessageID message_id = (bytes[0] << 8) | bytes[1 + 1];

    if (result->got_header) {
        result->payload.data.reply.ref_message_id = message_id;
    } else {
        result->payload.id = message_id;
        result->got_header = true;
    }

    return 2;
}

static size_t deserialize_udp_channel_id(const uint8_t *bytes, size_t size, PayloadDeserialization *result) {
    ChannelID buf = {0};
    ssize_t byte_read = scan_text(bytes, size, buf, CHANNEL_ID_LEN);
    if (byte_read < 0) return 0;

    // `join` is the only payload that has the `ChannelID` in its data field
    memcpy(result->payload.data.join.channel_id, buf, byte_read);

    return byte_read;
}

static size_t deserialize_udp_message_content(const uint8_t *bytes, size_t size, PayloadDeserialization *result) {
    MessageContent buf = {0};

    for (size_t i = 0; i < size; i++) {
        if (i > MESSAGE_CONTENT_LEN) {
            set_error(Error_InvalidPayload);
            return 0;
        }

        char ch = bytes[i];

        if (ch < 0x21 || ch > 0x7E) break;

        buf[i] = ch;

        // done
        if (ch == 0) {
            MessageContent *ref;
            switch (result->payload.type) {
                case PayloadType_Message:
                    ref = &result->payload.data.message.message_content;
                    break;
                case PayloadType_Reply:
                    ref = &result->payload.data.reply.message_content;
                    break;
                case PayloadType_Err:
                    ref = &result->payload.data.err.message_content;
                    break;
                default:
                    set_error(Error_InvalidPayload);
                    break;
            }

            memcpy(*ref, buf, i + 1);
            return i + 1;
        }
    }

    return 0;

}

static size_t deserialize_udp_username(const uint8_t *bytes, size_t size, PayloadDeserialization *result) {
    MessageContent buf = {0};
    ssize_t byte_read = scan_text(bytes, size, buf, MESSAGE_CONTENT_LEN);
    if (byte_read < 0) return 0;

    // `auth` is the only payload that has the `Username` in its data field
    memcpy(result->payload.data.auth.username, buf, byte_read);
    return byte_read;
}

static size_t deserialize_udp_display_name(const uint8_t *bytes, size_t size, PayloadDeserialization *result) {
    DisplayName buf = {0};

    for (size_t i = 0; i < size; i++) {
        if (i > DISPLAY_NAME_LEN) {
            set_error(Error_InvalidPayload);
            return 0;
        }

        char ch = bytes[i];

        if (ch < 0x20 || ch > 0x7E) break;

        buf[i] = ch;
        if (ch == 0) {
            DisplayName *ref;
            switch (result->payload.type) {
                case PayloadType_Auth:
                    ref = &result->payload.data.auth.display_name;
                    break;
                case PayloadType_Join:
                    ref = &result->payload.data.join.display_name;
                    break;
                case PayloadType_Message:
                    ref = &result->payload.data.message.display_name;
                    break;
                case PayloadType_Err:
                    ref = &result->payload.data.err.display_name;
                    break;
                default:
                    set_error(Error_InvalidPayload);
                    break;
            }

            memcpy(*ref, buf, i + 1);
            return i + 1;
        }
    }

    return 0;
}

static size_t deserialize_udp_secret(const uint8_t *bytes, size_t size, PayloadDeserialization *result) {
    Secret buf = {0};
    ssize_t byte_read = scan_text(bytes, size, buf, SECRET_LEN);
    if (byte_read < 0) return 0;

    // `auth` is the only payload that has the `Secret` in its data field
    memcpy(result->payload.data.auth.secret, buf, byte_read);
    return byte_read;
}

static size_t byte_count(uint8_t *bytes) {
    size_t i = 0;
    while (bytes[i++]) {}
    return i;
}

static void deserialization_state_push(uint32_t *current_state, enum DataType type) {
    int offset = 0;

    while (((*current_state >> offset) & 0x0F) == 0) {
        offset += 4;
    }

    *current_state |= (type << offset);
}

static enum DataType deserialization_state_get(uint32_t current_state) {
    return current_state & 0x0F;
}

static void deserialization_state_pop(uint32_t *current_state) {
    *current_state >>= 4;
}
