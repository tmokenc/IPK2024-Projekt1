#include "greatest.h"
#include "../src/trie.h"
#include <string.h>
#include <stdint.h>

Trie *TRIE;

static int trie_to_index(uint8_t ch) {
    if (ch < 'a' || ch > 'z') return -1;
    return ch - 'a';
}

static void trie_setup(void *arg) {
    TRIE = trie_new(trie_to_index);

    (void)arg;
}

static void trie_tear_down(void *arg) {
    trie_free(TRIE);
    (void)arg;
}


SUITE(trie);

TEST _trie_new() {
    ASSERT_EQ(trie_to_index, TRIE->to_index);
    ASSERT_EQ(TRIE->value, -1);

    for (int i = 0; i < TRIE_ARR_LEN; i++) {
        ASSERT_FALSE(TRIE->children[i]);
    }

    PASS();
}

TEST _trie_insert() {
    trie_insert(TRIE, (void *)"hi there", 1);
    trie_insert(TRIE, (void *)"he", 2);

    int h = trie_to_index('h');
    int i = trie_to_index('i');
    int e = trie_to_index('e');

    ASSERT_EQ(TRIE->value, -1);
    ASSERT(TRIE->children[h]);
    ASSERT_EQ(TRIE->children[h]->value, -1);
    ASSERT(TRIE->children[h]->children[i]);
    ASSERT_EQ(TRIE->children[h]->children[i]->value, 1);
    ASSERT_EQ(TRIE->children[h]->children[e]->value, 2);

    for (int j = 0; j < TRIE_ARR_LEN; j++) {
        ASSERT_FALSE(TRIE->children[h]->children[i]->children[j]);
        ASSERT_FALSE(TRIE->children[h]->children[e]->children[j]);
    }

    PASS();
}

TEST _trie_match_prefix() {
    char *data[] = {"auth", "rename", "hello", "help", "hellp", NULL};

    for (int i = 0; data[i]; i++) {
        trie_insert(TRIE, (void *)data[i], i);
    }

    ASSERT_EQ(trie_match_prefix(TRIE, (uint8_t *)"auth me"), 0);
    ASSERT_EQ(trie_match_prefix(TRIE, (uint8_t *)"rename me"), 1);
    ASSERT_EQ(trie_match_prefix(TRIE, (uint8_t *)"hello"), 2);
    ASSERT_EQ(trie_match_prefix(TRIE, (uint8_t *)"help me"), 3);
    ASSERT_EQ(trie_match_prefix(TRIE, (uint8_t *)"hellp us"), 4);
    ASSERT_EQ(trie_match_prefix(TRIE, (uint8_t *)"nothing to see here"), -1);

    PASS();
}

GREATEST_SUITE(trie) {
    GREATEST_SET_SETUP_CB(trie_setup, NULL);
    GREATEST_SET_TEARDOWN_CB(trie_tear_down, NULL);

    RUN_TEST(_trie_new);
    RUN_TEST(_trie_insert);
    RUN_TEST(_trie_match_prefix);
}
