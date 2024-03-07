/**
 * @file tcp.h
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 03/03/2024
 * @brief This module provides functions for TCP connection to be used in connection.h.
 */

#ifndef TCP_H
#define TCP_H

#include "connection.h"
#include <stdint.h>

/**
 * @brief Establish a non-blocing TCP connection to the specified host and port.
 * @param connection Pointer to the Connection object representing the connection.
 */
void tcp_connect(Connection *connection);

/**
 * @brief Send a payload over a TCP connection.
 * @param connection Pointer to the Connection object representing the TCP connection.
 * @param payload The payload to send.
 */
void tcp_send(Connection *connection, Payload payload);

/**
 * @brief Receive a payload over a TCP connection.
 * @param connection Pointer to the Connection object representing the TCP connection.
 * @return The received payload.
 */
Payload tcp_receive(Connection *connection);

/**
 * @brief Disconnect from a TCP connection.
 * @param connection Pointer to the Connection object representing the TCP connection.
 */
void tcp_disconnect(Connection *connection);

/**
 * @brief Get the next timeout for the poll function to wait
 * @param connection Pointer to the Connection object representing the UDP connection.
 * @return The timeout, -1 if not needed
 * @note this function will always return -1 since TCP connection does not need confirm on application layer
 */
int tcp_next_timeout(Connection *connection);

/// Serialize payload into bytes to be sent to the server
/// Exported for testing purpose
void tcp_serialize(const Payload *payload, Bytes *output);

/// Deserialize the incoming bytes into payload 
/// Exported for testing purpose
Payload tcp_deserialize(const uint8_t *bytes, size_t len);

#endif
