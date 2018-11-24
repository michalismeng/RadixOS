#ifndef MMNGR_AUX_VIRTUAL_H_28082018
#define MMNGR_AUX_VIRTUAL_H_28082018

#include <types.h>
#include <vmmngr_pde.h>
#include <vmmngr_pte.h>
#include <sync/spinlock.h>

// page table definition
typedef struct
{
	pt_entry entries[VIRT_MEM_PAGES_PER_TABLE];
} ptable_t;

// page directory definition
typedef struct
{
	pd_entry entries[VIRT_MEM_PAGES_PER_DIR];
} pdirectory_t;

// address space definition
typedef struct 
{
	physical_addr_t p_page_directory;				// physical address of the page directory
	spinlock_t lock;								// address space lock
} address_space_t;

// virtual memory recursive table
typedef virtual_addr_t vrec_table_t;

// returns the self recursive table that gives access to the entire page directory structure (pd_entries and pt_entries)
vrec_table_t virt_mem_get_self_recursive_table();

// returns the foreign recursive table that gives access to the page directory structure of another process (must be properly mapped first)
vrec_table_t virt_mem_get_foreign_recursive_table();

// returns the page table associated with the given address
ptable_t* virt_mem_get_page_table(vrec_table_t base, virtual_addr_t address);

// returns the directory based on the given address space
pdirectory_t* virt_mem_get_directory(vrec_table_t base);

// returns the current directory (based on the self recursive address)
pdirectory_t* virt_mem_get_current_directory();

// returns the foreign directory of the current directory (based on the foreign recursive address)
pdirectory_t* virt_mem_get_foreign_directory();

// returns an entry of pdirectory p based on addr
pd_entry* virt_mem_get_page_directory_entry(pdirectory_t* p, virtual_addr_t addr);

// returns the page directory entry for the given address
pd_entry* virt_mem_get_page_directory_entry_by_addr(vrec_table_t base, virtual_addr_t addr);

// returns the page table index in the directory that is associated with this address
uint32_t virt_mem_get_page_table_index(virtual_addr_t addr);

// returns the page table entry for the given address
pt_entry* virt_mem_get_page_table_entry_by_addr(vrec_table_t base, virtual_addr_t addr);

// returns an entry of ptable p based on addr
pt_entry* virt_mem_get_page_table_entry(ptable_t* p, virtual_addr_t addr);

// returns the page index in a page table that is associated with this address
uint32_t virt_mem_get_page_index(virtual_addr_t addr);


#endif