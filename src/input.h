/**
 * @file input.h
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 02/03/2024
 * @brief This module contains function to interact with the standard input
 */

#ifndef INPUT_H
#define INPUT_H

#include "bytes.h"
#include <stdio.h>

/**
 * @brief Read data from stdin and store it in the provided byte array.
 *
 * This function reads data from the standard input (stdin) and stores it in the provided
 * byte array, it also append the NULL byte at the end to be used as a normal C-string. 
 * The byte array should have enough capacity to store the read data, including
 * the ending byte (byte 0).
 *
 * @param bytes Pointer to the byte array to store the read data.
 * @return The number of bytes read (not including the NULL byte), or EOF.
 * @note It may set Error_InvalidInput if the input contains a 0 byte or exceeds the provided limit.
 */
int readLineStdin(Bytes *bytes);

#endif
