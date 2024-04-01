/**
 * @file args.h
 * @author Le Duy Nguyen (xnguye27)
 * @date 25/03/2024
 * @brief Definitions and functions related to command-line arguments parsing.
 */

#ifndef ARGS_H
#define ARGS_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Enumeration of communication modes.
 */
typedef enum {
    Mode_TCP,
    Mode_UDP,
} Mode;

/**
 * @brief Structure representing parsed command-line arguments.
 */
typedef struct {
    Mode mode; /**< Communication mode. */
    char *host; /**< Hostname or IP address. */
    uint16_t port; /**< Port number. */
    uint16_t udp_timeout; /**< UDP timeout value. */
    uint8_t udp_retransmissions; /**< Number of UDP retransmissions. */
    bool help; /**< Flag indicating whether help information should be displayed. */
} Args;

/**
 * @brief Parse command-line arguments and return the parsed arguments structure.
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 * @param mode Program mode (client or server).
 * @return Parsed command-line arguments structure.
 * @note This raise `Error_InvalidArgument` on failure.
 */
Args parse_args(int argc, char **argv);

#endif
