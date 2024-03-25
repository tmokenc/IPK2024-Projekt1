/**
 * @file error.h
 * @author Le Duy Nguyen (xnguye27)
 * @date 03/23/2024
 * @brief Definitions and functions related to error handling.
 */

#ifndef ERROR_H
#define ERROR_H

/**
 * @brief Enumeration of error types.
 */
typedef enum {
    Error_None, /**< No error. */
    Error_OutOfMemory, /**< Out of memory error. */
    Error_StackOverflow, /**< Stack overflow error. */
    Error_InvalidArgument, /**< Invalid argument error. */
    Error_DuplicatedArgument, /**< Duplicated argument error. */
    Error_Socket, /**< Socket-related error. */
    Error_Connection, /**< Connection-related error. */
    Error_InvalidPayload, /**< Invalid payload error. */
    Error_BadQuery, /**< Bad query error. */
    Error_InvalidInput /**< Invalid input error. */
} Error;

/**
 * @brief Set the current error type.
 * @param type The error type to set.
 */
void set_error(Error type);

/**
 * @brief Clear the current error.
 */
void error_clear();

/**
 * @brief Get the current error type.
 * @return The current error type.
 */
Error get_error();

#endif
