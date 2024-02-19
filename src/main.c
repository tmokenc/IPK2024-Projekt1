#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "error.h"
#include "client.h"
#include "args.h"
#include "input.h"

int main(int argc, char **argv) {
    Args args = parse_args(argc, argv);

    if (get_error()) {
        return get_error();
    }

    Client conn = connect(args);

    if (get_error()) {
        return get_error();
    }

    // Main loop
    while (1) {
        String input = readLineStdin(255);

        if (get_error()) {
            break;
        }

        connection_send(&conn, &input);

        if (get_error()) {
            string_free(&input);
            break;
        }

        String received = connection_receive(&conn);

        if (get_error()) {
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
