/**
 * @file main.c
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 25/03/2024
 * @brief The entry point of the program
 */

#include "error.h"
#include "client.h"
#include "server.h"
#include "args.h"
#include <string.h>
#include <stdio.h>

/**
 * @brief Get the current program mode (client / server)
 * @param program_name Name of the current executing program.
 * @return Program Mode to determine that the current program is client or server.
 */
ProgramMode current_program_mode(char *program_name);

/**
 * @brief Function pointer type for executing the program
 * @param args Arguments object to execute with
 */
typedef void (*Program)(Args args);

char *CLIENT_HELP = "Chat client implemented using IPK24chat protocol.\n"
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

char *SERVER_HELP = "Chat server implemented using IPK24chat protocol.\n"
"\n"
"Usage: ./ipk24chat-server [OPTIONS] \n"
"\n"
"Options:\n"
"  -l <IP>                  Server listening IP address for welcome sockets\n"
"  -p <PORT>                Server listening port for welcome sockets\n"
"  -d <number>              UDP confirmation timeout.\n"
"  -r <number>              Maximum number of UDP retransmissions.\n"
"  -h                       Print this message.\n";

int main(int argc, char **argv) {
    ProgramMode mode = current_program_mode(*argv);
    Program program = mode == ProgramMode_Server ? server_run : client_run;
    char *help = mode == ProgramMode_Server ? SERVER_HELP : CLIENT_HELP;

    Args args = parse_args(argc, argv, mode);

    if (get_error()) {
        fprintf(stderr, "%s", help);
        return get_error();
    }

    if (args.help) {
        printf("%s", help);
        return 0;
    }

    program(args);

    return get_error();
}

ProgramMode current_program_mode(char *program_name) {
    int len = strlen(program_name);
    int start = len - strlen("ipk24chat-server");
    /// If the program's name ends with ipk24chat-server, it's the server,  the client otherwise.
    return strcmp(program_name + start, "ipk24chat-server") == 0 
            ? ProgramMode_Server 
            : ProgramMode_Client;
}
