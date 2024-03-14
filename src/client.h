/**
 * @file client.h
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 14/03/2024
 * @brief This module provides structure and functions to work with the IPK2024 chat client.
 */

#ifndef CLIENT_H
#define CLIENT_H

#include "args.h"
#include "connection.h"
#include "payload.h"

/**
 * @brief Structure representing a chat client.
 */
typedef struct Client {
    Connection connection; /**< Connection to the server. */
    Secret secret; /**< Secret associated with the client. */
    DisplayName display_name; /**< Current user's display name */
    Username username; /**< Username of the client. */
} Client;

/**
 * @brief Initialize a chat client with the given arguments.
 * @param args Arguments containing client initialization information.
 * @return Initialized Client object.
 */
Client client_init(Args args);

/**
 * @brief Start the chat client.
 * @param client Pointer to the Client object representing the chat client.
 */
void client_start(Client *client);

/**
 * @brief Shutdown the chat client.
 * @param client Pointer to the Client object representing the chat client.
 */
void client_shutdown(Client *client);

#endif
