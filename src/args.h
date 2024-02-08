#ifndef ARGS_H
#define ARGS_H

typedef enum {
    ConnectionType_TCP,
    ConnectionType_UDP,
} ConnectionType;

typedef struct {
    ConnectionType type;
    unsigned port;
    char *host;
} Args;

Args parse_args(int argc, char **argv);

#endif
