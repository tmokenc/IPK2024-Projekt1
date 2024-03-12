/**
 * @file command.h
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 10/03/2024
 * @brief This module provides function and data structure for parsing user command
 */

#ifndef COMMAND_H
#define COMMAND_H

#include "connection.h"

typedef enum {
    CommandType_None, /*< no command*/

    CommandType_Auth,
    CommandType_Join,
    CommandType_Rename,
    CommandType_Help,

    // custom commands
    CommandType_Clear,
} CommandType;

typedef struct {
    DisplayName display_name;
    Username username;
    Secret secret;
} CommandAuthData;

typedef struct {
    ChannelID channel_id;
} CommandJoinData;

typedef struct {
    DisplayName display_name;
} CommandRenameData;

typedef struct {
    CommandType type;
    union {
        MessageContent message;
        CommandAuthData auth;
        CommandJoinData join;
        CommandRenameData rename;
    } data;
} Command;

void command_setup();

void command_clean_up();

/// Take in a null terminaated string and return the Command
/// this may set error into `InvalidInput`
Command command_parse(const uint8_t *str);

#endif
