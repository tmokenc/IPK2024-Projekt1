/**
 * @file udp.h
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 03/03/2024
 * @brief This module provides functions for UDP connection to be used in connection.h.
 */

#ifndef UDP_H
#define UDP_H

#include "connection.h"

/**
 * @brief Establish a UDP connection.
 * @param connection Pointer to the Connection object representing the UDP connection.
 */
void udp_connect(Connection *connection);

/**
 * @brief Send a payload over a UDP connection.
 * @param connection Pointer to the Connection object representing the UDP connection.
 * @param payload The payload to send.
 */
void udp_send(Connection *connection, Payload payload);

/**
 * @brief Receive a payload over a UDP connection.
 * @param connection Pointer to the Connection object representing the UDP connection.
 * @return The received payload.
 */
Payload udp_receive(Connection *connection);

/**
 * @brief Disconnect from a UDP connection.
 * @param connection Pointer to the Connection object representing the UDP connection.
 */
void udp_disconnect(Connection *connection);

/**
 * @brief Get the next timeout for the poll function to wait
 * @param connection Pointer to the Connection object representing the UDP connection.
 * @return the timeout, -1 if not needed
 */
int udp_next_timeout(Connection *connection);

/// Serialize payload into bytes to be sent to the server
/// Exported for testing purpose
Bytes udp_serialize(const Payload *payload);

/// Deserialize the incoming bytes into payload 
/// Exported for testing purpose
Payload udp_deserialize(Bytes buffer);

#endif
