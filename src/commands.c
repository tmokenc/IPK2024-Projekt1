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

static int start_non_whitespace(const uint8_t *str);
static int end_non_whitespace(const uint8_t *str);
static int command_char_to_index(const uint8_t ch);

static int strcpy_until_sp(uint8_t *dst, const uint8_t *src, int limit);

Trie *COMMAND_TRIE;

void command_setup() {
    // NULL at the ends as the indicator for end of the array
    char *commands[] = {"auth", "join", "rename", "help", "clear", NULL};
    char command_types[] = {
        CommandType_Auth,
        CommandType_Join,
        CommandType_Rename,
        CommandType_Help,
        CommandType_Clear,
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

    int start = start_non_whitespace(str);
    int end = end_non_whitespace(str);
    int len = end - start;

    if (len <= 0) {
        set_error(Error_InvalidInput);
        return cmd;
    }

    str += start;

    int maybe_command = str[0] == '/' ? trie_match_prefix(COMMAND_TRIE, str + 1) : -1;
    cmd.type = maybe_command <= 0 ? CommandType_None : maybe_command;
    
    switch (cmd.type) {
        case CommandType_None:
            memcpy(cmd.data.message, str, len);
            cmd.data.message[0] = 0;
            break;

        case CommandType_Auth:
            str += 6;
            str += strcpy_until_sp(cmd.data.auth.username, str, USERNAME_LEN) + 1;
            str += strcpy_until_sp(cmd.data.auth.secret, str, SECRET_LEN) + 1;
            str += strcpy_until_sp(cmd.data.auth.display_name, str, DISPLAY_NAME_LEN);
            break;
        case CommandType_Join:
            str += 6;
            strcpy_until_sp(cmd.data.join.channel_id, str, CHANNEL_ID_LEN);
            break;
        case CommandType_Rename:
            str += 8;
            strcpy_until_sp(cmd.data.rename.display_name, str, DISPLAY_NAME_LEN);
            break;

        case CommandType_Help:
        case CommandType_Clear:
            // No argument...
            break;
    }

    return cmd;
}

static int start_non_whitespace(const uint8_t *str) {
    int i = 0;

    while (str[i] == ' ') {
        i += 1;
    }

    return i;
}

static int end_non_whitespace(const uint8_t *str) {
    int len = strlen((void *)str);

    if (!len) {
        return 0;
    }

    int last_index = len - 1;

    while (str[last_index] == ' ') {
        last_index -= 1;
    }

    return last_index;
}

static int command_char_to_index(const uint8_t ch) {
    if (ch < 'a' || ch > 'z') return -1;
    return ch - 'a';
}

static int strcpy_until_sp(uint8_t *dst, const uint8_t *src, int limit) {
    int i = 0;

    while (src[i] != ' ' && src[i] != 0) {
        if (i + 1 > limit) {
            return -1;
        }

        dst[i] = src[i];
        i++;
    }

    dst[i + 1] = 0;

    return 0;
}
