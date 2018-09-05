#ifndef VM_CONTRACT_H_26102016
#define VM_CONTRACT_H_26102016

#include <types.h>
#include <vm_area.h>

// enum VM_CONTRACT_ERROR
// {
// 	VM_CONTRACT_NONE,
// 	VM_CONTRACT_BAD_ARGUMENTS,
// 	VM_CONTRACT_NOT_FOUND,
// 	VM_CONTRACT_BAD_AREA_ADDRESS,
// 	VM_CONTRACT_AREA_NON_REMOVABLE,
// 	VM_CONTRACT_OVERLAPS,
// 	VM_CONTRACT_AREA_EXISTS
// };

// defines the node of a vm_area for use in the interval tree
typedef struct vm_area_node
{
	vm_area_node* left, *right;				// left and regiht children
	uint32_t height;						// height of this node
	uint32_t max;							// maximum high address below this node

	vm_area_t area;

} vm_area_node_t;

// defines the 4 GB memory map of a process
typedef struct
{
	vm_area_node_t* root;			// vm contract is implemented as interval tree
} vm_contract_t;

// initializes a virtual memory contract. low is inclusive, high is exclusive
void vm_contract_init(vm_contract_t* c);

// inserts a new memory area that does not overlap with any other
error_t vm_contract_add_area(vm_contract_t* c, vm_area_t new_area);

// removes an area
error_t vm_contract_remove_area(vm_contract_t* c, vm_area_t* area);

// expands the given vm_area by 'length' if there is enough space, based on the GROWS_DOWN flag
error_t vm_contract_expand_area(vm_contract_t* c, vm_area_t* area, uint32_t length);

// returns the vm_area that contains the 'address'
vm_area_t* vm_contract_find_area(vm_contract_t* c, uint32_t address);

// returns a starting address that has 'length' length available. Caller constructs vm_area
virtual_addr_t vm_contract_get_area_for_length(vm_contract_t* c, uint32_t length);

// debug print the address contract
void vm_contract_print(vm_contract_t* c);

#endif