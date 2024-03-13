#include <stdio.h>
#include <signal.h>
#include "error.h"
#include "client.h"
#include "args.h"

int main(int argc, char **argv) {
    Args args = parse_args(argc, argv);

    if (get_error()) {
        return get_error();
    }

    Client client = client_init(args);

    if (get_error()) {
        return get_error();
    }

    client_start(&client);

    return get_error();
}
