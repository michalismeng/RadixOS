#ifndef CPU_MESSAGES_H_19112018
#define CPU_MESSAGES_H_19112018

// describes all the messages that can be sent to a cpu mailbox

typedef enum {
    CM_NONE,
    CM_RESCHEDULE,                  // triggers a reschedule on the target core
    CM_WAIT_AT_QUEUE,               // blocks the current thread at the given wait queue
} cpu_message_function_t;

#endif