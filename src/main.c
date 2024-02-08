#include <stdio.h>
#include "error.h"
#include "connection.h"
#include "args.h"
#include <string.h>
#include <stdlib.h>
#include <signal.h>

size_t readLineStdin(char *buf, size_t buf_size) {
    int ch;
    int count = 0;

    // the last char is reservated for the null terminator 
    while ((ch = fgetc(stdin) != EOF) && count < (buf_size - 1)) {
        if (ch == '\n') {
            break;
        }

        buf[count++] = ch;
    }

    buf[count] = '\0';
    return count;
}

int main(int argc, char **argv) {
    Args args = parse_args(argc, argv);

    if (error_get()) {
        return error_get();
    }

    Connection conn = connect(args);

    if (error_get()) {
        return error_get();
    }

    int size;
    char buf[1000];

    while ((size = readLineStdin(buf, 1000)) > 0) {
        connection_send(&conn, buf, size);

        if (error_get()) {
            /// OOps
            break;
        }

        size = connection_receive(&conn, buf, 1000);

        if (strcmp("bye", buf) == 0) {
            break;
        }

        printf("%s\n", buf);
    }

    disconnect(&conn);

    return error_get();
}
