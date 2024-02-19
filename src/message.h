#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdint.h>
typedef enum {
    /// Indicates that an error has occured while processing the other party's last message, 
    /// this eventually results in the termination of the communication
    MessageType_Err,

    ///  Only leveraged in certain protocol variants (UDP) 
    ///  to explicitly confirm the successful delivery of the message 
    ///  to the other party on the application level
    MessageType_Confirm,

    /// Some messages (requests) require a positive/negative confirmation 
    /// from the other side, this message contains such data
    MessageType_Reply,

    /// Used for client authentication (signing in) using user-provided username, 
    /// display name and a password
    MessageType_Auth,

    /// Represents client's request to join a chat channel by its identifier
    MessageType_Join,

    /// Contains user display name and a message designated for the channel they're joined in
    MessageType_Msg,

    /// Either party can send this message to indicate 
    /// that the conversation/connection is to be terminated. 
    /// This is the final message sent in a conversation 
    /// (except its potential confirmations in UDP)
    MessageType_Bye,
} MessageType;

typedef enum {
    ConnectionState_Start,
    ConnectionState_Auth,
    ConnectionState_Open,
    ConnectionState_Error,
    ConnectionState_End,
} ConnectionState;

typedef enum {
    MessageInputType_Auth,
    MessageInputType_Join,
    MessageInputType_Err,
    MessageInputType_Bye,
    MessageInputType_Msg,
    MessageInputType_Reply,
    MessageInputType_NotReply,
} MessageFmsType;

/// Max 20 bytes of [A-z0-0-]
#define USERNAME_LEN 20
/// Max 20 bytes of [A-z0-0-]
#define CHANNEL_ID_LEN 20
/// Max 128 bytes of [A-z0-0-]
#define SECRET_LEN 20
/// Max 128 bytes of printable characters (0x21-7E)
#define DISPLAY_NAME 128
/// Max 1400 bytes of printable characters with space (0x20-7E)
#define MESSAGE_CONTENT_LEN 1400

typedef uint16_t MessageID;
typedef char Username[USERNAME_LEN + 1];
typedef char ChannelID[CHANNEL_ID_LEN + 1];
typedef char Secret[SECRET_LEN + 1];
typedef char DisplayName[DISPLAY_NAME + 1];
typedef char MessageContent[MESSAGE_CONTENT_LEN + 1];

#endif
