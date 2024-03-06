/**
 * @file payload.c
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 02/03/2024
 * @brief Implementation of the payload.h
 */

#include "payload.h"

/// The ID of the next payload, this will be incremented each time the payload_new function is called
MessageID NEXT_MESSAGE_ID;

Payload payload_new(PayloadType type, PayloadData *data) {
    Payload payload;
    payload.type = type;
    payload.id = NEXT_MESSAGE_ID++;
    if (data) payload.data = *data;

    return payload;
}
