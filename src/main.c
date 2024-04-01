/**
 * @file main.c
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 25/03/2024
 * @brief The entry point of the program
 */

#include "error.h"
#include "args.h"
#include <string.h>
#include <stdio.h>

/**
 * @brief Function pointer type for executing the program
 * @param args Arguments object to execute with
 */
typedef void (*Program)(Args args);

#ifdef SERVER_F

#include "server.h"

char *HELP = "Chat server implemented using IPK24chat protocol.\n"
"\n"
"Usage: ./ipk24chat-server [OPTIONS] \n"
"\n"
"Options:\n"
"  -l <IP>                  Server listening IP address for welcome sockets\n"
"  -p <PORT>                Server listening port for welcome sockets\n"
"  -d <number>              UDP confirmation timeout.\n"
"  -r <number>              Maximum number of UDP retransmissions.\n"
"  -h                       Print this message.\n";

#else
#include "client.h"

char *HELP = "Chat client implemented using IPK24chat protocol.\n"
"\n"
"Usage: ./ipk24chat-client [OPTIONS] \n"
"\n"
"Options:\n"
"  -s <HOST>    (REQUIRED)  Server IP or hostname.\n"
"  -t <tcp|udp> (REQUIRED)  Transport protocol used for connection.\n"
"  -p <PORT>                Server port\n"
"  -d <number>              UDP confirmation timeout.\n"
"  -r <number>              Maximum number of UDP retransmissions.\n"
"  -h                       Print this message.\n";

#endif


int main(int argc, char **argv) {
    Args args = parse_args(argc, argv);

    if (get_error()) {
        fprintf(stderr, "%s", HELP);
        return get_error();
    }

    if (args.help) {
        printf("%s", HELP);
        return 0;
    }

    log("Start Program");
#ifdef SERVER_F
    server_run(args);
#else
    client_run(args);
#endif

    return get_error();
}
