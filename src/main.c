#include <stdio.h>
#include <signal.h>
#include "error.h"
#include "client.h"
#include "args.h"
#include <signal.h> 

bool SHOULD_SHUTDOWN = false;

char *HELP = "Chat client implemented using IPK24chat protocol.\n"
"\n"
"Usage: ./ipk24chat-client [OPTIONS] \n"
"\n"
"Options:\n"
"  -s <HOST>    (REQUIRED)  Server IP or hostname.\n"
"  -p <PORT>    (REQUIRED)  Server port\n"
"  -t <tcp|udp> (REQUIRED)  Transport protocol used for connection.\n"
"  -d <number>              UDP confirmation timeout.\n"
"  -r <number>              Maximum number of UDP retransmissions.\n"
"  -h                       Print this message.\n";

void handle_sigint(int sig) { 
    // The client should read 
    SHOULD_SHUTDOWN = true;
    (void)sig;
} 

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

    // Signal handle
    signal(SIGINT, handle_sigint); 

    Client client = client_init(args);

    if (get_error()) {
        return get_error();
    }

    client_run(&client);

    return get_error();
}
