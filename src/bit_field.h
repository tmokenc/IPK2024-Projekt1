/**
 * @file bit_field.h
 * @author Le Duy Nguyen (xnguye27)
 * @date 26/03/2024
 * @brief Definitions and functions related to a data structure designed for storing MessageIDs.
 */

#ifndef BIT_FIELD_H
#define BIT_FIELD_H

#include <stdbool.h>
#include "payload.h"

/**
 * @brief Structure representing a bit field.
 * 
 * This data structure is designed specifically for storing MessageIDs.
 */
typedef struct {
    uint8_t *data; /**< Pointer to the underlying data array. */
} BitField;

/**
 * @brief Create a new bit field.
 * @return A newly created BitField structure.
 */
BitField bit_field_new();

/**
 * @brief Free the memory allocated for a bit field.
 * @param bit_field Pointer to the BitField structure to free.
 */
void bit_field_free(BitField *bit_field);

/**
 * @brief Insert a MessageID into the bit field.
 * @param bit_field Pointer to the BitField structure.
 * @param msg_id The the MessageID to insert.
 */
void bit_field_insert(BitField *bit_field, MessageID msg_id);

/**
 * @brief Check if a MessageID is contained in the bit field.
 * @param bit_field Pointer to the BitField structure.
 * @param msg_id the MessageID to check for its existence in the field.
 * @return true if the MessageID is contained in the bit field, false otherwise.
 */
bool bit_field_contains(BitField *bit_field, MessageID msg_id);

#endif
