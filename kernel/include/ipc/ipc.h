#ifndef IPC_H_15112018
#define IPC_H_15112018

#include <ipc/message.h>
#include <ipc/mailbox.h>

// sends the given message. Both source and destination are embedded into the message fields.
error_t send(message_t* msg);

// receive a message from the given meailbox
error_t receive(mailbox_t* recv_mbox, message_t* msg);

// receive a message using the mailbox attached to the current thread
error_t receive_current(message_t* msg);

// acknowledges receipt of a CPU-only message. Must be called with a filled message after receive when type is CPU
error_t acknowledge(message_t* msg);


#endif