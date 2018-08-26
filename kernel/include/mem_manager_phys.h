#ifndef PHYS_MEM_MANAGER_H_15022018
#define PHYS_MEM_MANAGER_H_15022018

#include <types.h>

// initialize the physical memory manager. (size in KB)
void phys_mem_init(uint32_t size, physical_addr base);

// reserve a region
error_t phys_mem_reserve_region(uint32_t base, uint32_t length);

// free a region
error_t phys_mem_free_region(uint32_t base, uint32_t length);

// allocates a block of 4 KB that lies above the given address
physical_addr phys_mem_alloc_above(physical_addr addr);

// allocate a block of 4 KB
physical_addr phys_mem_alloc();

// allocate a block of 4 KB above the 1 MB mark
physical_addr phys_mem_alloc_above_1mb();

// deallocate a block of 4 KB
error_t phys_mem_dealloc(physical_addr blk);

uint32_t phys_mem_get_bitmap_size();

#endif