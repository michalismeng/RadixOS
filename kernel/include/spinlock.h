#ifndef SPINLOCK_H_28052018
#define SPINLOCK_H_28052018

#include <types.h>

/* Header prototypes for assembly function calls */

// acquires the given lock by spinning on it. lock must point to a 4-byte aligned integer
void acquire_lock(uint32_t* lock);

// releases the given lock immediatelly. lock must point to a 4-byte aligned integer
void release_lock(uint32_t* lock);

#endif