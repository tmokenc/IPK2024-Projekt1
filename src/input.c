/**
 * @file input.c
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 02/03/2024
 * @brief Implementation of the input.h
 */

#include "input.h"
#include "error.h"
#include <stdio.h>

int readLineStdin(uint8_t *bytes, size_t max_len) {
    if (feof(stdin)) return EOF;

    int ch;
    size_t index = 0;

    while ((ch = fgetc(stdin) != EOF) && ch != '\n') {
        if (index >= (max_len - 1)) {
            set_error(Error_InvalidInput);
            break;
        }

        bytes[index++] = ch;
    }

    bytes[index++] = 0;

    return index;
}
