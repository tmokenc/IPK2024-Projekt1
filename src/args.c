#include "args.h"
#include "error.h"
#include <string.h>
#include <stdio.h>

/// return -1 if in case of overflow (requires more than 16 bit) or not a number
int parse_16bit_number(char *str) {
    int num = 0;

    for (int i = 0; str[i]; i++) {
        if (str[i] < '0' || str[i] > '9') {
            return -1;
        }

        num *= 10;
        num += str[i] - '0';

        if (num > UINT16_MAX) {
            return -1;
        }
    }

    return num;
}

Args parse_args(int argc, char **argv) {
    Args args;

    if (argc == 2 && strcmp(argv[1], "-h") == 0) {
        args.help = true;
        return args;
    }

    if (argc % 2 == 0 || argc < 5) {
        set_error(Error_InvalidArgument);
        return args;
    }

    args.port = 4567;
    args.udp_timeout = 250;
    args.udp_retransmissions = 3;
    args.help = false;

    bool got_port = false;
    bool got_host = false;
    bool got_mode = false;
    bool got_timeout = false;
    bool got_udp_retransmissions = false;

    int idx = 1;
    
    while (idx < (argc - 1)) {
        char *key = argv[idx++];
        char *val = argv[idx++];

        if (strlen(key) != 2 && key[0] != '-') {
            set_error(Error_InvalidArgument);
            return args;
        }

        switch (key[1]) {
            case 's': {
                if (got_host) {
                    set_error(Error_DuplicatedArgument);
                    return args;
                }

                args.host = val;
                got_host = true;
                break;
            }

            case 'p': {
                if (got_port) {
                    set_error(Error_DuplicatedArgument);
                    return args;
                }

                int num = parse_16bit_number(val);
                if (num < 0) {
                    fprintf(stderr, "Port number should be in the range 0 to 65535\n");
                    set_error(Error_InvalidArgument);
                    return args;
                }

                args.port = num;
                got_port = true;
                break;
            }

            case 'm': {
                if (got_mode) {
                    set_error(Error_DuplicatedArgument);
                    return args;
                }

                if (strcmp(val, "udp") == 0) {
                    args.mode = Mode_UDP;
                } else if (strcmp(val, "tcp") == 0) {
                    args.mode = Mode_TCP;
                } else {
                    set_error(Error_InvalidArgument);
                    return args;
                }

                got_mode = true;
                break;
            }

            case 'd': {
                if (got_timeout) {
                    set_error(Error_DuplicatedArgument);
                    return args;
                }

                int num = parse_16bit_number(val);
                if (num < 0) {
                    fprintf(stderr, "UDP confirmation timeout should be in the range 0 to 65535\n");
                    set_error(Error_InvalidArgument);
                    return args;
                }

                args.udp_timeout = num;
                got_timeout = true;
                break;
            }
            case 'r': {
                if (got_udp_retransmissions) {
                    set_error(Error_DuplicatedArgument);
                    return args;
                }

                int num = parse_16bit_number(val);
                if (num < 0 || num > UINT8_MAX) {
                    fprintf(stderr, "UDP confirmation timeout should be in the range 0 to 255\n");
                    set_error(Error_InvalidArgument);
                    return args;
                }

                args.udp_retransmissions = num;
                got_udp_retransmissions = true;
                break;
            }

            default:
                set_error(Error_InvalidArgument);
                return args;
        }
    }

    if (!got_host) {
        set_error(Error_InvalidArgument);
    } else if (!got_mode) {
        set_error(Error_InvalidArgument);
    }

    return args;
}
