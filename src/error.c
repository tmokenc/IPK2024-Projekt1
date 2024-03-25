/**
 * @file error.c
 * @author Le Duy Nguyen (xnguye27)
 * @date 03/23/2024
 * @brief Implementation for error.h
 */

#include "error.h"

Error ERROR;

void set_error(Error error) {
    ERROR = error;
}

void error_clear() {
    ERROR = Error_None;
}

Error get_error() {
    return ERROR;
}
