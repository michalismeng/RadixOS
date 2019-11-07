#ifndef THREAD_MESSAGES_H_19112018
#define THREAD_MESSAGES_H_19112018

#include <types.h>

// describes all the messages that can be sent to a thread mailbox

typedef enum {
    TM_NONE,
    TM_FORK                         // fork the source process
} thread_message_function_t;

#endif