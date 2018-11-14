#include <semaphore.h>
#include <thread_sched.h>

void semaphore_init(sempahore_t* sem, uint32_t val)
{
    sem->count = val;
    sem->lock = 0;
    sem->waiting = 0;
}

void semaphore_wait(sempahore_t* sem)
{
    acquire_spinlock(sem->lock);

    if(sem->count <= 0)
    {
        release_spinlock(sem->lock);

        // Add thread to semaphore queue
        
        scheduler_block_running_thread();
    }
    else
        sem->count--;

    release_spinlock(sem->lock);
}

void semaphore_signal(sempahore_t* sem)
{
    acquire_spinlock(sem->lock);

    if(sem->count <= 0)
    {
        TCB* thread = sem->waiting;
        sem->waiting = thread->next;



        // scheduler_add_ready();

        // remove 'thread' from block queue and insert into ready queue
    }
    else
        sem->count++;

    release_spinlock(sem->lock);
}