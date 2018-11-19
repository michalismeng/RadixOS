#include <ipc/ipc.h>
#include <utility.h>
#include <sync/semaphore.h>
#include <thread_sched.h>
#include <lapic.h>
#include <gst.h>

error_t __send_mail_cpu(mailbox_t* dest, message_t* msg)
{
    acquire_spinlock(&dest->lock.msg_lock);

    memcpy(&dest->message, msg, sizeof(message_t));

    lapic_send_ipi_std(get_gst()->lapic_base, ((per_cpu_data_t*)dest->owner)->id, 100); // send messaging ipi

    return ERROR_OK;
}

error_t __send_mail_thread(mailbox_t* dest, message_t* msg)
{
    semaphore_wait(&dest->lock.sem_rdy);

    // copy message to destination kernel structure 
    memcpy(&dest->message, msg, sizeof(message_t));

    // signal the destination that he is ready to receive 
    semaphore_signal(&dest->lock.sem_recv);

    return ERROR_OK;
}

error_t __recv_mail_cpu(mailbox_t* mbox, message_t* msg)    // mbox is the receiver's mailbox
{
    memcpy(msg, &mbox->message, sizeof(message_t));

    return ERROR_OK;
}

error_t __recv_mail_thread(mailbox_t* mbox, message_t* msg)
{
    // wait for a message to be ready for consumption
    semaphore_wait(&mbox->lock.sem_recv);

    // copy message from kernel to user buffer
    memcpy(msg, &mbox->message, sizeof(message_t));

    // signal that the buffer is ready to receive a new message
    semaphore_signal(&mbox->lock.sem_rdy);

    return ERROR_OK;
}

error_t send(message_t* msg)
{
    mailbox_t* mbox = get_mailbox(msg->dst);

    if(!mbox)
        return ERROR_OCCUR;

    switch(mbox->type)
    {
        case MAILBOX_CPU: return __send_mail_cpu(mbox, msg); break;
        case MAILBOX_THREAD: return __send_mail_thread(mbox, msg); break;
        default: return ERROR_OCCUR;
    }

    return ERROR_OCCUR;
}

error_t receive(mailbox_t* recv_mbox, message_t* msg)
{
    if(!recv_mbox)
        return ERROR_OCCUR;

    switch(recv_mbox->type)
    {
        case MAILBOX_CPU: return __recv_mail_cpu(recv_mbox, msg); break;
        case MAILBOX_THREAD: return __recv_mail_thread(recv_mbox, msg); break;
        default: return ERROR_OCCUR;
    }

    return ERROR_OCCUR;
}

error_t acknowledge(message_t* msg)
{
    mailbox_t* mbox = get_mailbox(msg->dst);

    if(mbox->type != MAILBOX_CPU)
        return ERROR_OCCUR;

    release_spinlock(&mbox->lock.msg_lock);

    return ERROR_OK;
}