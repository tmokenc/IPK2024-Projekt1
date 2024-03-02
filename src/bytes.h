/**
 * @file bytes.h
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 02/03/2024
 * @brief This module provides data structures and functions for creating and managing
 * a slice of bytes that is dynamically allocated on the heap, which can be used as a buffer.
 */

#ifndef BYTES_H
#define BYTES_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Structure representing a slice of bytes.
 */
typedef struct {
    uint8_t *data;    /**< Pointer to the byte array */
    size_t capacity;  /**< Capacity of the byte array */
    size_t len;       /**< Length of the byte array */
} Bytes;

/**
 * @brief Constructs a new Bytes object.
 * @return A new Bytes object.
 */
Bytes bytes_new();

/**
 * @brief Destructs a Bytes object and frees its memory.
 * @param bytes The Bytes object to free.
 */
void bytes_free(Bytes *bytes);

/**
 * @brief Appends a byte to the end of the Bytes object.
 * @param bytes The Bytes object to which the byte will be appended.
 * @param byte The byte to append.
 */
void bytes_push(Bytes *bytes, uint8_t byte);

/**
 * @brief Reallocates the inner data array of Bytes.
 * @param bytes The Bytes object to reallocate.
 * @param new_capacity The new capacity of the inner data array. 
 *                     If -1, the capacity is adjusted to match the length.
 */
void bytes_realloc(Bytes *bytes, ssize_t new_capacity);
    
/**
 * @brief Appends data from another array to the end of the Bytes object.
 * @param bytes The Bytes object to which the data will be appended.
 * @param arr The array containing data to append.
 * @param n The number of elements in the array.
 */
void bytes_push_arr(Bytes *bytes, const uint8_t *arr, size_t n);

/**
 * @brief Removes the first n bytes from the Bytes object.
 * @param bytes The Bytes object from which bytes will be removed.
 * @param n The number of bytes to remove.
 * @note This function preserves the allocated memory. Use byte_realloc(bytes, -1) to truncate it.
 */
void bytes_remove_first_n(Bytes *bytes, size_t n);

#endif
