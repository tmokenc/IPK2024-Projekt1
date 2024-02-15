#include "args.h"
#include "error.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

/// return-1 for invalid port number
int parse_port_number(char *str) {
    int num = 0;

    for (int i = 0; str[i]; i++) {
        if (str[i] < '0' || str[i] > '9') {
            return -1;
        }

        num *= 10;
        num += str[i] - '0';

        if (num >= 1 << 16) {
            return -1;
        }
    }

    return num;
}

Args parse_args(int argc, char **argv) {
    Args args;

    bool got_port = false;
    bool got_host = false;
    bool got_type = false;

    if (argc != 7) {
        set_error(Error_InvalidArgument);
        return args;
    }

    int idx = 1;
    
    while (idx < (argc - 1)) {
        char *key = argv[idx++];
        char *val = argv[idx++];

        if (strcmp(key, "-h") == 0) {
            args.host = val;
            got_host = true;
        } else if (strcmp(key, "-p") == 0) {
            args.port = parse_port_number(val);
            if (args.port < 0) {
                fprintf(stderr, "Port number should be in the range 0 to 65535\n");
                set_error(Error_InvalidArgument);
                break;
            }

            got_port = true;
        } else if (strcmp(key, "-m") == 0) {
            if (strcmp(val, "udp") == 0) {
                args.type = ConnectionType_UDP;
            } else if (strcmp(val, "tcp") == 0) {
                args.type = ConnectionType_TCP;
            } else {
                set_error(Error_InvalidArgument);
                break;
            }

            got_type = true;
        } else {
            set_error(Error_InvalidArgument);
            break;
        }
    }

    if (!got_host) {
        set_error(Error_InvalidArgument);
    } else if (!got_port) {
        set_error(Error_InvalidArgument);
    } else if (!got_type) {
        set_error(Error_InvalidArgument);
    }

    return args;
}
