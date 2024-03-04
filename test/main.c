#include "greatest.h"

#include "args.c"
#include "bytes.c"
#include "payload.c"

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
    GREATEST_MAIN_BEGIN();

    RUN_SUITE(args);
    RUN_SUITE(bytes);
    RUN_SUITE(payload);

    GREATEST_MAIN_END();
}
