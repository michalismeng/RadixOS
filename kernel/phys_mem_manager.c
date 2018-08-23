#include <phys_mem_manager.h>
#include <utility.h>
#include <debug.h>

// private data

physical_addr mmb;      // address of the master memory block
uint16_t mmb_count;     // count of entries in the master memory block

#define MMB_CAPACITY 4096
#define MEM_NO_NEXT  (physical_addr)-1
#define is_page_aligned(x) is_aligned(x, 4096)

// public functions

int phys_mem_manager_init(physical_addr _mmb)
{
	if (!is_page_aligned(_mmb))
		return 1;

	mmb = _mmb;
	mmb_count = 0;

	return 0;
}

int phys_mem_insert_region(physical_addr start, uint32_t length)
{
	if (!is_page_aligned(start) || !is_page_aligned(length))
		return 1;

	if ((mmb_count + 1) * sizeof(mmb_entry_t) > MMB_CAPACITY)
		return 2;

	// Check that the memory region(start, length) does not interfere with(mmb, 4096)
    // mmb can be located at the first block of a region, hence the 'start < '
	if (start < mmb && mmb < start + length)   
		return 3;

	mmb_entry_t* new_region = (mmb_entry_t*)mmb + mmb_count++;
	new_region->next = start;
	new_region->next_size = length;

	new_region->mem_region_length = length;
	new_region->mem_region_start = start;

	// zero out the last chunk
	mem_region_t* last = (mem_region_t*)(start + length - sizeof(mem_region_t));
	last->next = MEM_NO_NEXT;
	last->next_size = 0;

	return 0;
}

physical_addr phys_mem_alloc_in_region(uint16_t index)
{
	mmb_entry_t* head = (mmb_entry_t*)mmb + index;

	if (head == 0 || index >= mmb_count)
		return 0;

	physical_addr ret_addr = 0;
	// if the block is bigger than a page => this is the main memory chunk
	if (head->next_size > 4096)
	{
		// the allocated address to be returned to the caller
		ret_addr = head->next;

		head->next = (int)head->next + 4096;
		head->next_size -= 4096;
		return ret_addr;
	}
	// this is a deallocated block
	else if (head->next_size == 4096)
	{
		ret_addr = head->next;
		mem_region_t* blk_target = ret_addr + 4096 - sizeof(mem_region_t);

		head->next = blk_target->next;
		head->next_size = blk_target->next_size;

		// erase block data for safety
		blk_target->next = 0;
		blk_target->next_size = 0;
		return ret_addr;
	}

	// out of memory
	return 0;
}

physical_addr phys_mem_alloc()
{
	physical_addr addr = 0;

	for (uint32_t i = 0; i < mmb_count; i++)
	{
		// try allocation in the i-th block
		if ((addr = phys_mem_alloc_in_region(i)) != 0)
			return addr;
	}

    // totally out of memory !!!
	return 0;
}

int phys_mem_dealloc_from_region(uint16_t index, physical_addr blk)
{
	mmb_entry_t* head = (mmb_entry_t*)mmb + index;

	if (head == 0 || blk == 0 || is_page_aligned(blk) == 0)
		return 1;

	mem_region_t* blk_region = blk + 4096 - sizeof(mem_region_t);
	blk_region->next = head->next;
	blk_region->next_size = head->next_size;

	head->next = blk;
	head->next_size = 4096;

	return 0;
}

int phys_mem_dealloc(physical_addr blk)
{
	if (blk == 0 || is_page_aligned(blk) == 0)
		return 1;

	for (int i = 0; i < mmb_count; i++)
	{
		mmb_entry_t* head = (mmb_entry_t*)mmb + i;

		// find a region which contains blk and deallocate there
		if ((physical_addr)head->mem_region_start <= blk && blk < (physical_addr)head->mem_region_start + head->mem_region_length)
			return phys_mem_dealloc_from_region(i, blk);
	}

	// memory map does not hold a region that contains blk
	return 2;
}

void phys_mem_print_region(mmb_entry_t* head)
{
	mem_region_t* temp = head;

	while (temp->next != MEM_NO_NEXT)
	{
		printfln("free block: %h to %h size %u", temp->next, temp->next + temp->next_size - 1, temp->next_size);
		temp = (int)temp->next + temp->next_size - sizeof(mem_region_t);
	}
}

void phys_mem_print()
{
	printfln("-------------------------");
    for(int i = 0; i < mmb_count; i++)
    {
        mmb_entry_t* head = (mmb_entry_t*)mmb + i;
		printfln("memory block at: %h to %h size %h", head->mem_region_start, head->mem_region_start + head->mem_region_length - 1, head->mem_region_length);
        phys_mem_print_region(head);
		printfln("-----------------");
    }
}

uint16_t phys_mem_get_mmb_index_for(physical_addr address)
{
	for(uint16_t i = 0; i < mmb_count - 1; i++)
	{
		mmb_entry_t* e = (mmb_entry_t*)mmb + i;

		if(address >= e->mem_region_start && address < (e + 1)->mem_region_start)
			return i;
	}

	return mmb_count - 1;
}