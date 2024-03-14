/**
 * @file input.c
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 02/03/2024
 * @brief Implementation of the input.h
 */

#include "input.h"
#include "error.h"
#include <stdio.h>

int readLineStdin(Bytes *bytes) {
    if (feof(stdin)) return EOF;

    int ch;
    size_t index = 0;

    while ((ch = fgetc(stdin) != EOF) && ch != '\n') {
        if (index >= (BYTES_SIZE - 1)) {
            set_error(Error_InvalidInput);
            break;
        }

        bytes_push(bytes, ch);
    }

    bytes_push(bytes, 0);

    return index;
}
