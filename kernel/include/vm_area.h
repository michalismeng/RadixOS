#ifndef VM_AREA_H_25102016
#define VM_AREA_H_25102016

#include <types.h>

// defines a process virtual area. (This is the bare bones of a process - kernel memory contract)
typedef struct
{
	uint32_t start_addr;			// area inclusive start address
	uint32_t end_addr;				// area inclusive end address (last valid address)
	uint32_t flags;					// area flags used to determine page fault action
	uint32_t fd;					// the global file descriptor connected with this area
	uint64_t offset;				// the offset withing the file of the mapping

} vm_area_t;


// creates a vm area
vm_area_t vm_area_create(uint32_t start, uint32_t end, uint32_t flags, uint32_t fd, uint64_t offset);

// returns whether a intersects with b vm_area_t
int vm_area_intersect(vm_area_t* a, vm_area_t* b);

// debug prints a vm_area_t
void vm_area_print(vm_area_t* area);

// returns the actual (not full-page) length of the area
uint32_t vm_area_get_length(vm_area_t* area);

// returns whether area can be removed from a contract.
int vm_area_is_removable(vm_area_t* area);

// return whether the area grows downwards
int vm_area_grows_down(vm_area_t* area);

// checks if the area is usable
int vm_area_is_ok(vm_area_t* area);

#endif