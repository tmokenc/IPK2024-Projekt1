#ifndef ARGS_H
#define ARGS_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    Mode_TCP,
    Mode_UDP,
} Mode;

typedef struct {
    Mode mode;
    char *host;
    uint16_t port;
    uint16_t udp_timeout;
    uint8_t udp_retransmissions;
    bool help;
} Args;

Args parse_args(int argc, char **argv);

#endif
