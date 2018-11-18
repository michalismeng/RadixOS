#ifndef MAILBOX_H_17112018
#define MAILBOX_H_17112018

#include <ipc/message.h>
#include <sync/semaphore.h>
#include <sync/spinlock.h>

#define MAX_MAILBOX_SLOTS (uint16_t)-1

typedef enum { 
    MAILBOX_NONE, 
    MAILBOX_CPU,
    __MAILBOX_ABOVE_SEMAPHORE__,        // all the above types use semaphores instead of spinlocks for message passing          
    MAILBOX_THREAD
} mailbox_type_t;

// #pragma pack(push, 1)

typedef struct {

    mid_t mid;                          // mailbox id
    uint16_t type;                      // mailbox type
    void* owner;                        // handle to the owner of this mailbox
    message_t message;					// message struct used to send and receive messages

    union {
        struct {
            semaphore_t sem_rdy;					// owner is ready to receive data
            semaphore_t sem_recv;					// owner is allowed to consume the message (aka message is fully copied to local buffer)
        };

        spinlock_t msg_lock;                    // message is in use. This is set by the sender (before the ipi) and released by the owner after consuming the message.
    } lock;

} mailbox_t;

// #pragma pack(pop)


void mailbox_init();
mailbox_t* mailbox_create_static(mid_t mid, mailbox_type_t type, void* owner);
mailbox_t* mailbox_create(mailbox_type_t type, void* owner);

mailbox_t* get_mailbox(mid_t mid);

#endif