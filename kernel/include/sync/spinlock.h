#ifndef SPINLOCK_H_28052018
#define SPINLOCK_H_28052018

#include <types.h>

typedef uint32_t spinlock_t;

/* Header prototypes for assembly function calls */

// acquires the given lock by spinning on it. lock must point to a 4-byte aligned integer
void acquire_spinlock(spinlock_t* lock);

// acquires the given lock and disables local cpu interrupts
void acquire_spinlock_irq(spinlock_t* lock);

// acquires the given lock, disables local cpu interrupts and returns the previous flags
uint32_t acquire_spinlock_irqsave(spinlock_t* lock);

// releases the given lock immediatelly. lock must point to a 4-byte aligned integer
void release_spinlock(spinlock_t* lock);

// releases the given lock and enables local cpu interrupts
void release_spinlock_irq(spinlock_t* lock);

// releases the given lock, enables local cpu interrupts and restores flags
void release_spinlock_irqrestore(spinlock_t* lock, uint32_t flags);

#endif