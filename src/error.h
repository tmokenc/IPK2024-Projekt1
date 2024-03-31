/**
 * @file error.h
 * @author Le Duy Nguyen (xnguye27)
 * @date 03/23/2024
 * @brief Definitions and functions related to error handling.
 */

#ifndef ERROR_H
#define ERROR_H

#ifdef DEBUG_F

/**
 * @brief macro to print out debug string if NDEBUG flag is disabled
 **/
#define log(s) fprintf(stderr, "- "__FILE__":%u: "s"\n", __LINE__)

/**
 * @brief macro to print out debug string formatted (like printf) if NDEBUG flag is disabled
 **/
#define logfmt(s, ...) fprintf(stderr, "- "__FILE__":%u: "s"\n",__LINE__,__VA_ARGS__)

#else

#define log(s)
#define logfmt(s, ...) 

#endif

/**
 * @brief macro to print error message to stdout
 */
#define eprint(s) fprintf(stderr, "ERR: " s "\n")

/**
 * @brief macro to print error message to stdout with formatting like printf
 */
#define eprintf(s, ...) fprintf(stderr, "ERR: " s "\n", __VA_ARGS__)

/**
 * @brief Enumeration of error types.
 */
typedef enum {
    Error_None, /**< No error. */
    Error_OutOfMemory, /**< Out of memory error. */
    Error_StackOverflow,
    Error_InvalidArgument, /**< Invalid argument error. */
    Error_DuplicatedArgument, /**< Duplicated argument error. */
    Error_Socket, /**< Socket-related error. */
    Error_Connection, /**< Connection-related error. */
    Error_InvalidPayload, /**< Invalid payload error. */
    Error_RecvFromWrongAddress, /**< Receive from a server address we are not listening to. */
    Error_BadQuery, /**< Bad query error. */
    Error_InvalidInput, /**< Invalid input error. */
    Error_Internal /**< Internal error*/
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
