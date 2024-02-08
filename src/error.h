#ifndef ERROR_H
#define ERROR_H

typedef enum {
    None,
    ErrorType_OutOfMemory,
    ErrorType_InvalidArgument,
    ErrorType_Connection,
    ErrorType_BadQuery,
} ErrorType;

void error_set(ErrorType type);

ErrorType error_get();

#endif
