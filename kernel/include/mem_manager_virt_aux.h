#ifndef MMNGR_AUX_VIRTUAL_H_28082018
#define MMNGR_AUX_VIRTUAL_H_28082018

#include <types.h>
#include <vmmngr_pde.h>
#include <vmmngr_pte.h>

typedef struct page_table
{
	pt_entry entries[VIRT_MEM_PAGES_PER_TABLE];
} ptable_t;

// page directory definition
typedef struct page_directory
{
	pd_entry entries[VIRT_MEM_PAGES_PER_DIR];
} pdirectory_t;

// address space is defined as the base virtual address used for the recursion mapping. 
// The address space has access to every table and the page directory.
typedef virtual_addr_t address_space_t;

// returns the current address space
address_space_t virt_mem_get_current_address_space();

// returns the page table associated with the given address
ptable_t* virt_mem_get_page_table(address_space_t base, virtual_addr_t address);

// returns the directory based on the given address space
pdirectory_t* virt_mem_get_directory(address_space_t base);

// returns the current directory (based on address 0xC0000000)
pdirectory_t* virt_mem_get_current_directory();

// returns entry of pdirectory p based on addr
pd_entry* virt_mem_get_page_directory_entry(pdirectory_t* p, virtual_addr_t addr);

// returns entry of ptable p based on addr
pt_entry* virt_mem_get_page_table_entry(ptable_t* p, virtual_addr_t addr);

// returns the page table index in the directory that is associated with this address
uint32_t virt_mem_get_page_table_index(virtual_addr_t addr);

// returns the page index in a page table that is associated with this address
uint32_t virt_mem_get_page_index(virtual_addr_t addr);


#endif