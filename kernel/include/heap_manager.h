#ifndef HEAP_MANAGER_H_20022018
#define HEAP_MANAGER_H_20022018

#include <types.h>

#define HEAP_BLOCK_MAGIC 0x1F2E

// Heap header block definition. (One per requested allocation)
typedef struct heap_block_struct_t
{
	uint16_t magic;
	uint8_t used;
	uint8_t flags;
	struct heap_block_struct_t* next;
} heap_block_t;

// Heap master. Defines a heap_t region to allocate blocks and space for user.
typedef struct heap_struct_t
{
	void* start_address;
	uint32_t size;
	uint32_t current_blocks;
} heap_t;

// creates a heap_t of size at the virtual address base.
heap_t* heap_create(void* base, uint32_t size);

// allocates size bytes at the heap_t h for use by a user program.
// also merges every contiguous unused blocks found on its way.
void* heap_alloc(heap_t* h, uint32_t size);

// deallocates a previously allocated space. (May front-merge unused blocks)
int heap_free(heap_t* h, void* address);

// re-allocates a previously allocated space to take up 'new_size' space.
void* heap_realloc(heap_t* h, void* address, uint32_t new_size);

// defrags the heap_t h merging all contiguous unsued blocks.
uint32_t heap_defrag(heap_t* h);

// allocates size bytes at the heap_t that are aligned.
void* heap_alloc_a(heap_t* h, uint32_t r_size, uint32_t align);

// deallocated a previously allocated and aligned space.
int heap_free_a(heap_t* h, void* address);

// displays the heap_t master entry along with info for every block allocated.
void heap_print(heap_t* h);

#endif