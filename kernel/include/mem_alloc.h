#ifndef MEM_ALLOC_H_08092018
#define MEM_ALLOC_H_08092018

#include <types.h>

// allocate static virtual memory (non-freeable). allocation begins at the end of the kernel image
virtual_addr_t alloc_perm();

#endif