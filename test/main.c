#include "greatest.h"

#include "error.c"
#include "args.c"
#include "bytes.c"
#include "payload.c"
#include "tcp.c"
#include "udp.c"
#include "trie.c"
#include "commands.c"

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
    GREATEST_MAIN_BEGIN();

    RUN_SUITE(error);
    RUN_SUITE(args);
    RUN_SUITE(bytes);
    RUN_SUITE(payload);
    RUN_SUITE(tcp);
    RUN_SUITE(udp);
    RUN_SUITE(trie);
    RUN_SUITE(commands);

    GREATEST_MAIN_END();
}
