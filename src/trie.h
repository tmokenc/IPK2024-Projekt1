/**
 * @file trie.h
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 11/03/2024
 * @brief This module provides simple Trie data structure made exclusively for this project
 */

#ifndef TRIE_H
#define TRIE_H

#include <stdint.h>

/// 27 because of the 26 alphabet characters and space
#define TRIE_ARR_LEN 27 /**< Length of the children array in the Trie node. */

/**
 * @brief Function pointer type to convert a byte into an index.
 *
 * This function pointer type is used to convert a byte into an index for Trie node children.
 * It should return -1 if conversion is not possible.
 */
typedef int (*ToIndexFunc)(uint8_t);

/**
 * @brief Structure representing a Trie node.
 */
typedef struct trie {
    int value; /**< Value associated with the Trie node. -1 if it does not contain any value. */
    ToIndexFunc to_index; /**< Custom function to convert a byte into an index. */
    struct trie *children[TRIE_ARR_LEN]; /**< Array of pointers to child Trie nodes. */
} Trie;

/**
 * @brief Create a new Trie node.
 * @param func Function pointer to convert a byte into an index.
 * @return Pointer to the newly created Trie node.
 * @note if the note cannot be created, it return NULL and set error to Error_OutOfMemory
 */
Trie *trie_new(ToIndexFunc func);

/**
 * @brief Free memory allocated for the Trie and its children.
 * @param trie Pointer to the Trie node to free.
 */
void trie_free(Trie *trie);

/**
 * @brief Insert a string (prefix) with a value into the Trie.
 * @param trie Pointer to the root Trie node.
 * @param str Pointer to the string (prefix) to insert.
 * @param value Value associated with the string (prefix).
 */
void trie_insert(Trie *trie, const uint8_t *str, int value);

/**
 * @brief Match a prefix in the Trie and return its associated value.
 * @param trie Pointer to the root Trie node.
 * @param str Pointer to the string (prefix) to match.
 * @return The value associated with the matched prefix, or -1 if no match is found.
 */
int trie_match_prefix(Trie *trie, const uint8_t *str);

#endif
