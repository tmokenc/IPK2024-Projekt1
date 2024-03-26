/**
 * @file connection.h
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 03/03/2024
 * @brief This module provides data structures and functions to interact with the server using the IPK2024 protocol.
 */

#ifndef CONNECTION_H
#define CONNECTION_H

#include "args.h"
#include "payload.h"
#include <stdint.h>
#include <sys/socket.h>

/**
 * @brief Structure representing a connection to the server.
 */
typedef struct Connection Connection;

/**
 * @brief Function pointer type for connecting to the server.
 * @param connection Pointer to the Connection structure representing the connection.
 */
typedef void (*ConnectFunc)(Connection *connection);

/**
 * @brief Function pointer type for sending a payload to the server.
 * @param connection Pointer to the Connection structure representing the connection.
 * @param payload The payload to send.
 */
typedef void (*SendFunc)(Connection *connection, Payload payload);

/**
 * @brief Function pointer type for receiving a payload from the server.
 * @param connection Pointer to the Connection structure representing the connection.
 * @return The received payload.
 */
typedef Payload (*ReceiveFunc)(Connection *connection);

/**
 * @brief Function pointer type for disconnecting from the server.
 * @param connection Pointer to the Connection structure representing the connection.
 * @note this function does not have to close the socket
 */
typedef void (*DisconnectFunc)(Connection *connection);

/**
 * @brief Structure representing a connection to the server.
 */
struct Connection {
    Args args; /**< Application arguments. */
    int sockfd; /**< Socket file descriptor for the connection. */
    struct addrinfo *address_info; /**< Info about host address. */

    ConnectFunc connect; /**< Function pointer for connecting to the server. */
    SendFunc send; /**< Function pointer for sending data to the server. */
    ReceiveFunc receive; /**< Function pointer for receiving data from the server. */
    DisconnectFunc disconnect; /**< Function pointer for disconnecting from the server. */
};

/**
 * @brief Initialize a Connection object with the given arguments.
 * @param args Arguments containing connection information.
 * @return Initialized Connection object.
 */
Connection connection_init(Args args);

/**
 * @brief Close a Connection object.
 * @param connection Pointer to the Connection object to destroy.
 */
void connection_close(Connection *connection);

#endif
