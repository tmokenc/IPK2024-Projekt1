/**
 * @file bytes.h
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 02/03/2024
 * @brief This module provides data structures and functions for creating and managing
 * a slice of bytes, which can be used as a buffer.
 */

#ifndef BYTES_H
#define BYTES_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// The size of the bytes, as in specification, a payload cannot exceed 1500 bytes
#define BYTES_SIZE 1501

/**
 * @brief Structure representing a slice of bytes. This behaves like a C-string,
 * but with additional operatoions built into it
 */
typedef struct {
    uint8_t data[BYTES_SIZE];    /**< Pointer to the byte array */
    size_t len;       /**< Length of the byte array */
    size_t offset;    /**< Offset of the start of the bytes*/
} Bytes;

/**
 * @brief Constructs a new Bytes object.
 * @return A new Bytes object.
 */
Bytes bytes_new();

/**
 * @brief Get the inner bytes array correct offset that has been set using bytes_trim or bytes_remove_first_n
 * @param bytes The Bytes object
 * @return Pointer to the inner bytes array
 * @note always use this instead of accessing the internal bytes.data array
 */
const uint8_t *bytes_get(const Bytes *bytes);

/**
 * @brief Appends a byte to the end of the Bytes object.
 * @param bytes The Bytes object to which the byte will be appended.
 * @param byte The byte to append.
 */
void bytes_push(Bytes *bytes, uint8_t byte);

/**
 * @brief Clear the inner data, this is just an alias to memset(bytes, 0, sizeof(Bytes))
 * @param bytes The Bytes object to which the byte will be cleared.
 */
void bytes_clear(Bytes *bytes);

/**
 * @brief Appends data from another array to the end of the Bytes object.
 * @param bytes The Bytes object to which the data will be appended.
 * @param arr The array containing data to append.
 * @param n The number of elements in the array.
 */
void bytes_push_arr(Bytes *bytes, const uint8_t *arr, size_t n);

/**
 * @brief Append a null-terminated C string to the Bytes.
 * @param bytes Pointer to the Bytes object to which the string will be appended.
 * @param str Pointer to the null-terminated C string to append.
 */
void bytes_push_c_str(Bytes *bytes, const char *str);

/**
 * @brief Trim the bytes in both size with the given character
 * @params bytes The Bytes object o which will be trimmed
 * @params ch The character will be trimmed from both side
 * @return How many character it has been trimmed
 */
size_t bytes_trim(Bytes *bytes, uint8_t ch);

/**
 * @brief Skip the first n bytes from the Bytes object.
 * @param bytes The Bytes object
 * @param n The number of bytes to skipped.
 */
void bytes_skip_first_n(Bytes *bytes, size_t n);

#endif
