#include <ipc/ipc.h>
#include <utility.h>
#include <sync/semaphore.h>
#include <thread_sched.h>

error_t send(message_t* msg)
{
    // TODO: assert source and destination are sane

    // wait for the target thread to be receiving (ready to accept a new message)
    semaphore_wait(&get_thread(msg->dst)->msg_lock);

    // copy message to destination kernel structure 
    memcpy(&get_thread(msg->dst)->message, msg, sizeof(message_t));

    // signal the destination that we are ready to receive 
    semaphore_signal(&get_thread(msg->dst)->recv_sem);

    return ERROR_OK;
}

error_t receive(message_t* msg)
{
    // wait for a message to be ready for consumption
    semaphore_wait(&get_current_thread()->recv_sem);

    // copy message from kernel to user buffer
    memcpy(msg, &get_current_thread()->message, sizeof(message_t));

    // signal that the buffer is ready to receive a new message
    semaphore_signal(&get_current_thread()->msg_lock);

    // at this point the user can parse the message and consume it

    return ERROR_OK;
}
