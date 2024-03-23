/**
 * @file payload.c
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 02/03/2024
 * @brief Implementation of the payload.h
 */

#include "payload.h"
#include <string.h>

/// The ID of the next payload, this will be incremented each time the payload_new function is called
MessageID NEXT_MESSAGE_ID;

Payload payload_new(PayloadType type, PayloadData *data) {
    Payload payload;
    payload.type = type;
    payload.id = NEXT_MESSAGE_ID++;
    if (data) memcpy(&payload.data, data, sizeof(PayloadData));

    return payload;
}

static bool is_valid_id(uint8_t ch) {
    return ch == '-'
        || (ch >= 'a' && ch <= 'z')
        || (ch >= 'A' && ch <= 'Z')
        || (ch >= '0' && ch <= '9');
}

static bool is_valid_display_name(uint8_t ch) {
    return ch >= 0x21 && ch <= 0x7e;
}

static bool is_valid_message_content(uint8_t ch) {
    return ch >= 0x20 && ch <= 0x7e;
}

typedef bool (*Validator)(uint8_t ch);

static ssize_t read(uint8_t *dest, const Bytes *src, int limit, Validator validator) {
    const uint8_t *bytes = bytes_get(src);
    int count = 0;

    for (size_t i = 0; i < src->len; i++) {
        if (!validator(bytes[i])) break;
        if (count >= limit) return -1;
        count += 1;
    }

    if (count) {
        memcpy(dest, bytes, count);
        dest[count] = 0;
    }

    return count;
}

ssize_t read_username(Username dest, const Bytes *src) {
    return read(dest, src, USERNAME_LEN, is_valid_id);
}

ssize_t read_channel_id(ChannelID dest, const Bytes *src) {
    return read(dest, src, CHANNEL_ID_LEN, is_valid_id);
}

ssize_t read_secret(Secret dest, const Bytes *src) {
    return read(dest, src, SECRET_LEN, is_valid_id);
}

ssize_t read_display_name(DisplayName dest, const Bytes *src) {
    return read(dest, src, DISPLAY_NAME_LEN, is_valid_display_name);
}

ssize_t read_message_content(MessageContent dest, const Bytes *src) {
    return read(dest, src, MESSAGE_CONTENT_LEN, is_valid_message_content);
}
