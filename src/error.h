#ifndef ERROR_H
#define ERROR_H

typedef enum {
    Error_None,
    Error_OutOfMemory,
    Error_StackOverflow,
    Error_InvalidArgument,
    Error_DuplicatedArgument,
    Error_Socket,
    Error_Connection,
    Error_InvalidPayload,
    Error_BadQuery,
    Error_InvalidInput
} Error;

void set_error(Error type);

Error get_error();

#endif
