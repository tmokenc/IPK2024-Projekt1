/**
 * @file payload.h
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 02/03/2024
 * @brief This module provides data structures and functions to interact with the payload that is going to be sent or received from the server.
 */

#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdint.h>
#include "args.h"
#include "bytes.h"

#define USERNAME_LEN 20
#define CHANNEL_ID_LEN 20
#define SECRET_LEN 128
#define DISPLAY_NAME_LEN 20
#define MESSAGE_CONTENT_LEN 1400

// Define data types for payload components
typedef uint16_t MessageID;
typedef char Username[USERNAME_LEN + 1];
typedef char ChannelID[CHANNEL_ID_LEN + 1];
typedef char Secret[SECRET_LEN + 1];
typedef char DisplayName[DISPLAY_NAME_LEN + 1];
typedef char MessageContent[MESSAGE_CONTENT_LEN + 1];

typedef enum {
    PayloadType_Confirm = 0x00,
    PayloadType_Reply = 0x01,
    PayloadType_Auth = 0x02,
    PayloadType_Join = 0x03,
    PayloadType_Message = 0x04,
    PayloadType_Err = 0xFE,
    PayloadType_Bye = 0xFF,
} PayloadType;

// Define payload structures

typedef struct {
    ChannelID channel_id;
    DisplayName display_name;
} PayloadJoin;

typedef struct {
    bool result;
    MessageID ref_message_id;
    MessageContent message_content;
} PayloadReply;

typedef struct {
    Username username;
    DisplayName display_name;
    Secret secret;
} PayloadAuth;

typedef struct {
    DisplayName display_name;
    MessageContent message_content;
} PayloadMessage;

typedef struct {
    DisplayName display_name;
    MessageContent message_content;
} PayloadErr;

typedef union {
    PayloadJoin join;
    PayloadReply reply;
    PayloadAuth auth;
    PayloadMessage message;
    PayloadErr err;
} PayloadData;

typedef struct {
    PayloadType type;      /**< Type of the payload */
    PayloadData data;      /**< Data of the payload */
    MessageID id;          /**< MessageID as in IPK2024 specification */
} Payload;

typedef struct {
    uint32_t current_progress; /**< Current progress of deserialization, 0 means it has been fully deserialized */
    Payload payload;           /**< Current payload */
    Bytes leftover;            /**< Leftover bytes for next payload */
    bool got_header;           /**< If the header has been deserialized */
} PayloadDeserialization;

/**
 * @brief Create a new payload with the specified type and data.
 * @param type The type of the payload.
 * @param data Pointer to the data of the payload. NULL if it not need any
 * @return The new payload.
 * @note this will automatically assign an ID into it.
 *       The ID will start from 0 and is incremented each time this function is called
 */
Payload payload_new(PayloadType type, PayloadData *data);

/**
 * @brief Serialize a payload into bytes for transmission.
 * @param payload The payload to serialize.
 * @param mode The mode of transmission (TCP/UDP).
 * @return The serialized payload as bytes.
 */
Bytes payload_serialize(Payload *payload, Mode mode);

/**
 * @brief Deserialize a sequence of bytes into a `Payload`.
 * @param buf The buffer containing the bytes to deserialize.
 * @param len The length of the buffer.
 * @param mode The mode (TCP/UDP) for representing data.
 * @param result Pointer to the previous (or fresh new) `PayloadDeserialization` structure.
 * @note if the result->current_progress is 0 after calling this function, 
 *       it means the payload is ready to use.
 */
void payload_deserialize(const uint8_t *buf, size_t len, Mode mode, PayloadDeserialization *result);

#endif
