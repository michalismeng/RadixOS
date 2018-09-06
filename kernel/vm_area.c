#include <vm_area.h>
#include <utility.h>
#include <mem_manager_virt.h>
#include <debug.h>

// private functions

// checks that noth start and end addresses are page-aligned and that they follow the basic ineequality
int vm_area_check_bounds(uint32_t start, uint32_t end)
{
	if (start % virt_mem_get_page_size() != 0 ||
		(end - start + 1) % virt_mem_get_page_size() != 0 ||
		start >= end)
		return 0;

	return 1;
}

// public functions

vm_area_t vm_area_create(uint32_t start, uint32_t end, uint32_t flags, uint32_t fd, uint64_t offset)
{
	vm_area_t a;
	memset(&a, 0, sizeof(a));

	// assert good arguments and fail if necessary
	if (!vm_area_check_bounds(start, end))
	{
		// set_last_error(EINVAL, VM_AREA_BAD_BOUNDS, EO_VM_AREA);
		// return a bad area
		return a;	
	}

	a.start_addr = start;
	a.end_addr = end;
	a.flags = flags;
	a.fd = fd;
	a.offset = offset;

	return a;
}

int vm_area_intersect(vm_area_t* a, vm_area_t* b)
{
	if(a->end_addr < b->start_addr || a->start_addr > b->end_addr)
		return 0;

	return 1;
}

void vm_area_print(vm_area_t* area)
{
	printfln("%h - %h, length: %u bytes", area->start_addr, area->end_addr, vm_area_get_length(area));
}

uint32_t vm_area_get_length(vm_area_t* area)
{
	return area->end_addr - area->start_addr + 1;
}

int vm_area_is_removable(vm_area_t* area)
{
	return 1; //((area->flags & MMAP_NON_REMOVE) != MMAP_NON_REMOVE);
}

int vm_area_grows_down(vm_area_t* area)
{
	return 0; //((area->flags & MMAP_GROWS_DOWN) == MMAP_GROWS_DOWN);
}

int vm_area_is_ok(vm_area_t* area)
{
	if(/*((area->flags & MMAP_INVALID) != MMAP_INVALID) &&*/ area->start_addr < area->end_addr)
		return 1;
	return 0;
}