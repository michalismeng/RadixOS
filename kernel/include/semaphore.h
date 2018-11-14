#ifndef SEMAPHORE_H_14112018
#define SEMAPHORE_H_14112018

#include <types.h>
#include <spinlock.h>
#include <process.h>

typedef struct {

    uint32_t count;
    spinlock_t lock;

    TCB* waiting;

} sempahore_t;

// initializes the semaphore to the given value
void semaphore_init(sempahore_t* sem, uint32_t val);

// wait for the semaphore to be ready (count > 0)
void semaphore_wait(sempahore_t* sem);

// releases a used semaphore (count++)
void semaphore_signal(sempahore_t* sem);

#endif