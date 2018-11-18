#ifndef SEMAPHORE_H_14112018
#define SEMAPHORE_H_14112018

#include <types.h>
#include <sync/spinlock.h>
// #include <process.h>

typedef struct {

    spinlock_t lock;
    uint32_t count;

    struct thread_control_block* waiting_head;
    struct thread_control_block* waiting_tail;

} semaphore_t;


// initializes the semaphore to the given value
void semaphore_init(semaphore_t* sem, uint32_t val);

// wait for the semaphore to be ready (count > 0)
void semaphore_wait(semaphore_t* sem);

// releases a used semaphore (count++)
void semaphore_signal(semaphore_t* sem);

#endif