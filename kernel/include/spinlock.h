#ifndef SPINLOCK_H_28052018
#define SPINLOCK_H_28052018

#include <types.h>

typedef uint32_t spinlock_t;

/* Header prototypes for assembly function calls */

// acquires the given lock by spinning on it. lock must point to a 4-byte aligned integer
void acquire_spinlock(spinlock_t* lock);

// releases the given lock immediatelly. lock must point to a 4-byte aligned integer
void release_spinlock(spinlock_t* lock);

#endif