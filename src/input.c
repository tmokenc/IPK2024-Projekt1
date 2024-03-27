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
    log("Reading user input from stdin");
    if (feof(stdin)) return EOF;

    int ch;
    int count = 0;

    while ((ch = fgetc(stdin)) != EOF && ch != '\n') {
        if (count++ >= BYTES_SIZE) {
            set_error(Error_InvalidInput);
            continue;
        }

        bytes_push(bytes, ch);
    }

    if (ch == EOF) {
        log("Got EOF");
        return EOF;
    }

    if (!get_error()) {
        bytes_push(bytes, 0);
    }

    return count;
}
