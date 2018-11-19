#ifndef MESSAGE_H_14112018
#define MESSAGE_H_14112018

#include <types.h>
#include <ipc/cpu_messages.h>

#define CUSTOM_MESSAGE_LENGTH 64 - 6

typedef struct {
    void* ptr;
} msg_single_ptr_t;

typedef struct {
    void* ptr1;
    void* ptr2;
} msg_double_ptr_t;

typedef struct {

    mid_t src, dst;                                 // sender and receiver mailboxed

    uint8_t func;                                   // requested function
    uint8_t type;                                   // type of requested function

    union {                                         // types of all messages
        
        msg_single_ptr_t msg_ptr1;
        msg_double_ptr_t msg_ptr2;

        uint8_t msg_custom[CUSTOM_MESSAGE_LENGTH];      // fixes the size to 64 bytes
    } payload;

} message_t;

#endif