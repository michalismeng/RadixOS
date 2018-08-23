#include <heap_manager.h>
#include <debug.h>

// private functions

uint32_t heap_block_t_size(heap_t* h, heap_block_t* b)
{
	if (b->next == 0)
		return (uint32_t)h->start_address + h->size - (uint32_t)b - sizeof(heap_block_t);
	return (uint32_t)b->next - (uint32_t)b - sizeof(heap_block_t);
}

void* heap_block_t_start_address(heap_block_t* b)
{
	return ((char*)b + sizeof(heap_block_t));
}

// merges block b with its next. No condition checks are made
void heap_front_merge_block(heap_t* h, heap_block_t* b)
{
	b->next = b->next->next;
	h->current_blocks--;
}

// returns true if the next block of prev is unused
int heap_is_next_unused(heap_block_t* prev)
{
	return (prev->next != 0 && prev->next->used == 0);
}

heap_block_t* heap_block_t_create(heap_t* h, void* base, int used, heap_block_t* next, int flags)
{
	heap_block_t* block = (heap_block_t*)base;
	block->magic = HEAP_BLOCK_MAGIC;
	block->used = used;
	block->next = next;
	block->flags = flags;

	h->current_blocks++;

	return block;
}

// Front merges blocks until either the merged block size suits r_size or fail.
// Merged block remains for on-spot defrag purposes.
int heap_front_merge_size_fit(heap_t* h, heap_block_t* prev, uint32_t r_size)
{
	while (prev->next != 0 && r_size > heap_block_t_size(h, prev))
	{
		if (heap_is_next_unused(prev))
			heap_front_merge_block(h, prev);
		else
			break;
	}

	return r_size <= heap_block_t_size(h, prev);
}

// public functions

heap_t* heap_create(void* base, uint32_t size)
{
	heap_t* new_heap = (heap_t*)base;
	new_heap->start_address = (char*)base + sizeof(heap_t);
	new_heap->size = size - sizeof(heap_t);
	new_heap->current_blocks = 0;

	heap_block_t_create(new_heap, (void*)new_heap->start_address, 0, 0, 0);
	return new_heap;
}

void* heap_alloc(heap_t* h, uint32_t r_size)
{
	if (h == 0 || r_size == 0)
		return 0;

	heap_block_t* block = (heap_block_t*)h->start_address;

	while (block != 0)
	{
		if (block->magic != HEAP_BLOCK_MAGIC)
			return 0;

		if (block->used == 1)
			block = block->next;
		else if (heap_front_merge_size_fit(h, block, r_size))	// try create contiguous chunk of at least 'r_size' size
		{
			uint32_t size = heap_block_t_size(h, block);

			if (r_size + sizeof(heap_block_t) < size)			// try squeeze a heap_block_t
			{
				heap_block_t* new_block = heap_block_t_create(h, (char*)block + sizeof(heap_block_t) + r_size, 0,
					block->next, block->flags);

				block->next = new_block;
			}

			block->used = 1;
			return heap_block_t_start_address(block);
		}
		else											// requested size does not fit so continue to the next block
			block = block->next;						// the contiguous unsused chunk remains.
	}

	// allocation failed
	return 0;
}

int heap_free(heap_t* h, void* address)
{
	if (h == 0 || address == 0)
	{
		return 1;
	}

	heap_block_t* block = (heap_block_t*)((char*)address - sizeof(heap_block_t));
	if (block->magic != HEAP_BLOCK_MAGIC)
		return 1;

	block->used = 0;
	if (block->next != 0 && block->next->used == 0)		// front-only merge unused block to partially defrag
	{
		block->next = block->next->next;
		h->current_blocks--;
	}

	return 0;
}

//void* heap_realloc(heap_t* h, void* address, uint32_t new_size)
//{
//	if (h == 0 || address == 0)
//	{
//		set_last_error(EINVAL, HEAP_BAD_ARGUMENT, EO_HMMNGR);
//		return 0;
//	}
//
//	heap_block_t* block = (heap_block_t*)((char*)address - sizeof(heap_block_t));
//
//	if (block->magic != heap_block_t_MAGIC)
//	{
//		set_last_error(EINVAL, HEAP_BAD_MAGIC, EO_HMMNGR);
//		return 0;
//	}
//
//	uint32_t block_size = heap_block_t_size(h, block);
//
//	if (block_size == new_size)
//		return address;
//
//	/*if (new_size < block_size)		// request to shrink allocated space
//	{
//	// try squeeze a new block
//	if (heap_is_next_unused(block))			// only one front merge required to squeeze a block
//	heap_front_merge_block(h, block);
//
//	uint32_t remaining_size = heap_block_t_size(h, block) - new_size;	// be careful as block size may have changed
//
//	if (remaining_size > sizeof(heap_block_t))		// squeeze a new block
//	{
//	heap_block_t* new_block = heap_block_t_create(h, (char*)address + new_size, false, block->next, block->flags);
//	block->next = new_block;
//	}
//
//	return heap_block_t_start_address(block);
//	}
//	else if (new_size > block_size)
//	{
//	if (heap_front_merge_size_fit(h, block, new_size))
//	{
//	}
//	}
//	else
//	return address;		// re-allocation does nothing as 'new_size' equals the currently allocated 'block_size'*/
//
//#ifndef HEAP_REALLOC_FIRST_IMPLEMENT
//	//uint32_t block_size = heap_block_t_size(h, block);
//
//	void* new_addr = heap_alloc(h, new_size);
//	if (new_addr == 0)
//		return 0;
//
//	memcpy(new_addr, address, min(block_size, new_size));
//	if (heap_free(h, address) != ERROR_OK)
//		return 0;
//
//	return new_addr;
//#endif
//}

uint32_t heap_defrag(heap_t* h)
{
	if (h == 0)
	{
        return 0;
	}

	uint32_t blocks_merged = 0;
	heap_block_t* block = (heap_block_t*)h->start_address;

	while (block != 0)
	{
		if (block->used == 0 && block->next != 0 && block->next->used == 0)		// merge blocks
		{
			block->next = block->next->next;
			h->current_blocks--;
			blocks_merged++;
		}
		else
			block = block->next;
	}

	return blocks_merged;
}

void* heap_alloc_a(heap_t* h, uint32_t r_size, uint32_t align)
{
	uint8_t* mem = heap_alloc(h, r_size + align + 4);

	uint32_t padding = 0;
	if((uint32_t)(mem + 4) % align != 0)
		padding = align - (uint32_t)(mem + 4) % align;

	uint8_t* ret_ptr = (mem + 4) + padding;
	*(uint32_t*)(ret_ptr - 4) = padding;

	return ret_ptr;
}

int heap_free_a(heap_t* h, void* address)
{
	uint32_t* addr = (uint32_t*)address;
	uint32_t pad = *(addr - 1);
	void* block = (uint8_t*)addr - pad - 4;

	return heap_free(h, block);
}

void heap_print(heap_t* h)
{
	if (h == 0)
		return;
	printfln("Heap start: %h with size: %h. Currently using %u blocks.\n", h->start_address, h->size, h->current_blocks);
	heap_block_t* block = (heap_block_t*)h->start_address;

	while (block != 0)
	{
		if (block->magic != HEAP_BLOCK_MAGIC)
			printf("HEAP ERROR");
		uint32_t size = heap_block_t_size(h, block);
		printf("block at: %h with size: %h, ", block, size);
		if (block->used)
			printf("used\n");
		else
			printf("unused\n");

		block = block->next;
	}
}