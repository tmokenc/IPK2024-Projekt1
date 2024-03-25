/**
 * @file command.c
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 10/03/2024
 * @brief Implementation of command.h
 */

#include "commands.h"
#include "error.h"
#include "trie.h"
#include <string.h>

#define TRIE_STARTS_AT 'a'
#define TRIE_ARR_SIZE 26

static int command_char_to_index(const uint8_t ch);
static int command_prefix_length(CommandType type);

Trie *COMMAND_TRIE;

void command_setup() {
    // NULL at the ends as the indicator for end of the array
    char *commands[] = {"auth", "join", "rename", "help", "clear", "exit", NULL};
    char command_types[] = {
        CommandType_Auth,
        CommandType_Join,
        CommandType_Rename,
        CommandType_Help,
        CommandType_Clear,
        CommandType_Exit,
    };

    COMMAND_TRIE = trie_new(command_char_to_index);
    if (get_error()) return;

    for (int i = 0; commands[i]; i++) {
        trie_insert(COMMAND_TRIE, (uint8_t *)commands[i], command_types[i]);
    }
}

void command_clean_up() {
    trie_free(COMMAND_TRIE);
}

Command command_parse(const uint8_t *str) {
    Command cmd = {0};

    Bytes buffer = bytes_new();
    bytes_push_c_str(&buffer, (const char *)str);
    bytes_trim(&buffer, ' ');

    if (buffer.len <= 0) {
        set_error(Error_InvalidInput);
        return cmd;
    }

    const uint8_t *slice = bytes_get(&buffer);

    int maybe_command = *slice == '/' ? trie_match_prefix(COMMAND_TRIE, slice + 1) : -1;
    cmd.type = maybe_command <= 0 ? CommandType_None : maybe_command;

    int cmd_offset = command_prefix_length(cmd.type);

    // set the bytes offset based of the command
    if (slice[cmd_offset] == ' ') {
        bytes_skip_first_n(&buffer, cmd_offset + 1);
    } else if (slice[cmd_offset] == 0) {
        bytes_skip_first_n(&buffer, cmd_offset);
    } else {
        cmd.type = CommandType_None;
    }

    int read = 0;

    #define READ(select, buf) \
        read = read_##buf(cmd.data.select.buf, &buffer); \
        if (read <= 0) { \
            set_error(Error_InvalidInput); \
            return cmd; \
        } \
        bytes_skip_first_n(&buffer, read)

    #define SKIP_SPACE() \
        if (*bytes_get(&buffer) != ' ') { \
            set_error(Error_InvalidInput); \
            return cmd; \
        } \
        bytes_skip_first_n(&buffer, 1)

    switch (cmd.type) {
        case CommandType_None:
            read = read_message_content(cmd.data.message, &buffer);
            if (read <= 0) { 
                set_error(Error_InvalidInput); 
                return cmd; 
            } 
            bytes_skip_first_n(&buffer, read);

            break;

        case CommandType_Auth:
            READ(auth, username); SKIP_SPACE();
            READ(auth, display_name); SKIP_SPACE();
            READ(auth, secret); 
            break;
        case CommandType_Join:
            READ(join, channel_id);
            break;
        case CommandType_Rename:
            READ(rename, display_name);
            break;

        case CommandType_Help:
        case CommandType_Clear:
        case CommandType_Exit:
            // No argument...
            break;
    }

    if (buffer.len != 0) {
        set_error(Error_InvalidInput);
    }

    return cmd;
}

static int command_prefix_length(CommandType type) {
    switch (type) {
        case CommandType_None:
            return 0;
        case CommandType_Auth:
        case CommandType_Join:
        case CommandType_Help:
        case CommandType_Exit:
            return 5;
        case CommandType_Clear:
            return 6;
        case CommandType_Rename:
            return 7;
    }

    return 0;
}

static int command_char_to_index(const uint8_t ch) {
    if (ch < 'a' || ch > 'z') return -1;
    return ch - 'a';
}
