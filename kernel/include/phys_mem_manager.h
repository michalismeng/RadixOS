#ifndef PHYS_MEM_MANAGER_H_15022018
#define PHYS_MEM_MANAGER_H_15022018

#include <types.h>

typedef struct mem_region_struct_t
{
	void* next;					    // the address of the next block of available memory
	physical_addr next_size;		// the size of the next available block of memory (if this block has been deallocated => size = 4096)

} mem_region_t;

typedef struct mmb_entry_struct_t
{
	void* next;
	uint32_t next_size;

	void* mem_region_start;			// the memory region start address
	uint32_t mem_region_length;		// the memory region length				(after beign set these two members stay constant)

	// TODO: Perhaps add region descriptions and enable the user to select a region in which to be allocated

} mmb_entry_t;

// initialize the master memory block address and count
int phys_mem_manager_init(physical_addr mmb_base);

// insert a memory region in the master memory block
int phys_mem_insert_region(physical_addr start, uint32_t length);

// allocate a block of 4 KB in the selected region
physical_addr phys_mem_alloc_in_region(uint16_t index);

// allocate a block of 4 KB in the first available region
physical_addr phys_mem_alloc();

// deallocate a block of 4 KB and return it to the selected region (the block address must lie in the selected region, no such check is made)
int phys_mem_dealloc_from_region(uint16_t index, physical_addr blk);

// deallocate a block of 4 KB and return it to its proper region
int phys_mem_dealloc(physical_addr blk);

void phys_mem_print_region(mmb_entry_t* head);
void phys_mem_print();

#endif