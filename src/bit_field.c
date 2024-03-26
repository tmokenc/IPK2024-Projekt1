/**
 * @file bit_field.
 * @author Le Duy Nguyen (xnguye27)
 * @date 26/03/2024
 * @brief Implementation for `bit_field.h`
 */

#include "bit_field.h"
#include "error.h"

BitField bit_field_new() {
    /// 2^(sizeof MessageID in bit) / 8
    size_t len = 1 << ((sizeof(MessageID) * 8) - 3);
    BitField result;

    result.data = malloc(len);

    if (!result.data) {
        set_error(Error_OutOfMemory);
    }

    return result;
}

void bit_field_free(BitField *bit_field) {
    free(bit_field->data);
}

void bit_field_insert(BitField *bit_field, MessageID msg_id) {
    size_t index = msg_id / 8;
    size_t offset = msg_id % 8;
    bit_field->data[index] |= (1 << offset);
}

bool bit_field_contains(BitField *bit_field, MessageID msg_id) {
    size_t index = msg_id / 8;
    size_t offset = msg_id % 8;
    return (bit_field->data[index] >> offset) & 1;
}
