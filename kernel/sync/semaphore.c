#include <sync/semaphore.h>
#include <thread_sched.h>
#include <gst.h>
#include <ipc/ipc.h>

void semaphore_init(semaphore_t* sem, uint32_t val)
{
    sem->count = val;
    sem->lock = 0;
    sem->waiting_head = sem->waiting_tail = 0;
}

void semaphore_wait(semaphore_t* sem)
{
    acquire_spinlock(&sem->lock);

    if(sem->count <= 0)
    {
        message_t msg;
        msg.dst = get_cpu_id;
        msg.src = get_cpu_id;
        msg.func = CM_SEM_WAIT;                 // send the SEMAPHORE WAIT message
        msg.payload.msg_ptr1.ptr = sem;

        send(&msg);
        return;         // the spinlock is released by the scheduler prior to sleeping => just return
    }
    else
        sem->count--;

    release_spinlock(&sem->lock);
}

void semaphore_signal(semaphore_t* sem)
{
    acquire_spinlock(&sem->lock);

    if(sem->count <= 0)
    {
        TCB* thread;

        if(sem->waiting_head == sem->waiting_tail)
        {
            thread = sem->waiting_head;
            sem->waiting_head = sem->waiting_tail = 0;
        }
        else
        {
            thread = sem->waiting_head;
            sem->waiting_head = thread->next;
            thread->next = 0;
        }

        release_spinlock(&sem->lock);

        message_t msg;
        msg.dst = thread->exec_cpu;
        msg.src = get_cpu_id;
        msg.func = CM_AWAKEN_THREAD;
        msg.payload.msg_ptr1.ptr = thread;
        send(&msg);
        
        return;
    }
    else
        sem->count++;

    release_spinlock(&sem->lock);
}