#ifndef CONNECTION_H
#define CONNECTION_H

#include "string.h"
#include "args.h"
#include <stdint.h>

#define USERNAME_LEN 20
#define CHANNEL_ID_LEN 20
#define SECRET_LEN 128
#define DISPLAY_NAME_LEN 20
#define MESSAGE_CONTENT_LEN 1400

typedef uint16_t MessageID;
typedef char Username[USERNAME_LEN + 1];
typedef char ChannelID[CHANNEL_ID_LEN + 1];
typedef char Secret[SECRET_LEN + 1];
typedef char DisplayName[DISPLAY_NAME_LEN + 1];
typedef char MessageContent[MESSAGE_CONTENT_LEN + 1];

struct Connection;

typedef void (*ErrFunc)(struct Connection *);
typedef void (*ConfirmFunc)(struct Connection *);
typedef void (*ReplyFunc)(struct Connection *, bool, MessageContent);
typedef void (*AuthFunc)(struct Connection *, Username, DisplayName, Secret);
typedef void (*JoinFunc)(struct Connection *, ChannelID, DisplayName);
typedef void (*MsgFunc)(struct Connection *, DisplayName, MessageContent);
typedef void (*ByeFunc)(struct Connection *);

typedef String (*ReceiveFunc)(struct Connection *);
typedef int (*ConnectFunc)(char *, uint16_t);
typedef void (*DisconnectFunc)(struct Connection *);

typedef struct Connection {
    int sockfd;

    AuthFunc auth;
    JoinFunc join;
    ReplyFunc reply;
    ConfirmFunc confirm;
    MsgFunc msg;
    ErrFunc err;
    ByeFunc bye;

    ConnectFunc connect;
    ReceiveFunc receive;
    DisconnectFunc disconnect;
} Connection;

Connection connection_init(Args);

void connection_destroy(Connection *);

#endif
