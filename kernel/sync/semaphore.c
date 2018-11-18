#include <sync/semaphore.h>
#include <thread_sched.h>
#include <gst.h>

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
        TCB* cur = scheduler_current_remove_running();

        if(sem->waiting_tail == 0)
            sem->waiting_head = sem->waiting_tail = cur;
        else
        {
            sem->waiting_tail->next = cur;
            sem->waiting_tail = cur;
            cur->next = 0;
        }

        release_spinlock(&sem->lock);

        scheduler_current_run_thread();
        return;
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
        printfln("removing from queue: %h", sem->waiting_head);
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
        scheduler_add_ready(&get_cpu_storage(thread->exec_cpu)->scheduler, thread);
        return;
    }
    else
        sem->count++;

    release_spinlock(&sem->lock);
}