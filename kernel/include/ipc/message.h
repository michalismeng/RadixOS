#ifndef MESSAGE_H_14112018
#define MESSAGE_H_14112018

#include <types.h>

#define CUSTOM_MESSAGE_LENGTH 64 - 6

typedef struct {

    mid_t src, dst;                                 // sender and receiver mailboxed

    uint8_t func;                                   // requested function
    uint8_t type;                                   // type of requested function

    union {                                         // types of all messages
        uint32_t custom_int;
        uint8_t custom[CUSTOM_MESSAGE_LENGTH];
    } payload;

} message_t;

#endif