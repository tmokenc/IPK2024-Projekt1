/**
 * @file trie.c
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 11/03/2024
 * @brief Implementation of trie.h
 */

#include "trie.h"
#include "error.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Trie *trie_new(ToIndexFunc func) {
    Trie *trie = malloc(sizeof(Trie));

    if (!trie) {
        set_error(Error_OutOfMemory);
        return NULL;
    }

    logfmt("Allocating %lu bytes for trie", sizeof(Trie));
    memset(trie, 0, sizeof(Trie));
    trie->to_index = func;
    trie->value = -1;

    return trie;
}

void trie_free(Trie *trie) {
    if (!trie) {
        return;
    }

    for (int i = 0; i < TRIE_ARR_LEN; i++) {
        // Recursively free the Trie
        trie_free(trie->children[i]);
    }

    free(trie);
}

void trie_insert(Trie *trie, const uint8_t *str, int value) {
    int index = trie->to_index(str[0]);

    if (index < 0) {
        // Insert value to the Trie that can not be advanded deeper with the given string
        trie->value = value;
        return;
    }

    // Ensure that the sub-trie is always valid
    if (!trie->children[index]) {
        Trie *sub_trie = trie_new(trie->to_index);
        if (get_error()) return;
        trie->children[index] = sub_trie;
    }

    // Recursively go to the end of the str to insert the value
    trie_insert(trie->children[index], str + 1, value);
}

int trie_match_prefix(Trie *trie, const uint8_t *str) {
    // As for this project, the root trie is always valid, no need to check for its existence
    while (1) {
        int index = trie->to_index(*str);

        // either it cannot turn into the index or the sub-trie not contains that
        if (index < 0 || !trie->children[index]) {
            return trie->value;
        }

        trie = trie->children[index];
        str += 1;
    }
}
