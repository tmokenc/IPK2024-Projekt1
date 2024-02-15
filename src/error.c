#include "error.h"

Error ERROR;

void set_error(Error error) {
    ERROR = error;
}

Error get_error() {
    return ERROR;
}
