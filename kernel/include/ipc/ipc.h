#ifndef IPC_H_15112018
#define IPC_H_15112018

#include <ipc/message.h>

// sends the given message. Both source and destination are embedded into the message fields.
error_t send(message_t* msg);

// receive a message
error_t receive(message_t* msg);

#endif