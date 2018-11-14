#ifndef MESSAGE_H_14112018
#define MESSAGE_H_14112018

#include <types.h>

#define CUSTOM_MESSAGE_LENGTH 64 - 10

typedef struct {

    pid_t sender_proc, recv_proc;                   // sender and receiver process
    tid_t sender_thread, recv_thread;               // sender and receiver thread

    uint16_t func;                                  // requested function
    uint16_t type;                                  // type of requested function

    union {                                         // types of all messages

        uint8_t custom[CUSTOM_MESSAGE_LENGTH];
    } payload;

} message_t;

#endif