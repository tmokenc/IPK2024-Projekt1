#include "error.h"

ErrorType ERROR;


void error_set(ErrorType error) {
    ERROR = error;
}

ErrorType error_get() {
    return ERROR;
}
