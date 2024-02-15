#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "error.h"
#include "connection.h"
#include "args.h"
#include "input.h"

int main(int argc, char **argv) {
    Args args = parse_args(argc, argv);

    if (get_error()) {
        return get_error();
    }

    Connection conn = connect(args);

    if (get_error()) {
        return get_error();
    }

    String input;

    while (1) {
        input = readLineStdin(255);

        if (get_error()) {
            // OOPS
            break;
        }

        connection_send(&conn, &input);

        if (get_error()) {
            // OOPS
            break;
        }

        String received = connection_receive(&conn);

        if (get_error()) {
            // OOPS
            string_free(&input);
            string_free(&received);
            break;
        }

        if (strcmp("bye", received.data) == 0) {
            string_free(&input);
            string_free(&received);
            break;
        }

        printf("%s\n", received.data);

        string_free(&input);
        string_free(&received);
    }

    disconnect(&conn);

    return get_error();
}
