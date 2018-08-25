#ifndef MMNGR_VIRTUAL_H_130516
#define MMNGR_VIRTUAL_H_130516

#include <types.h>
#include <isr.h>

#include <vmmngr_pte.h>
#include <vmmngr_pde.h>

// TODO: Move these to some virtual memory definitions...
// page table definition
typedef struct page_table
{
	pt_entry entries[VIRT_MEM_PAGES_PER_TABLE];
} ptable;

// page directory definition
typedef struct page_directory
{
	pd_entry entries[VIRT_MEM_PAGES_PER_DIR];
} pdirectory;

#define PAGE_SIZE 4096

#define VIRT_MEM_DEFAULT_PDE_FLAGS I86_PDE_PRESENT | I86_PDE_WRITABLE	// default flags for page tables.
#define VIRT_MEM_DEFAULT_PTE_FLAGS I86_PTE_PRESENT | I86_PTE_WRITABLE	// default flags for page entries.

// definitions for entry extraction based on virtual address. (see virtual address format)

#define PAGE_DIR_INDEX(x)			( ((x) >> 22) & 0x3ff )		// Get the 10 upper bits of x
#define PAGE_TABLE_INDEX(x)			( ((x) >> 12) & 0x3ff )		// Get the 10 "middle" bits of x
#define PAGE_GET_PHYSICAL_ADDR(x)	( (*x) & ~0xfff )			// Physical address is 4KB aligned, so return all bits except the 12 first

void page_fault(iregisters_t* regs);

// USEFUL DATA CHAIN cr3 -> pdirectory -> (PAGE_DIR_INDEX(v_addr)) -> pd_entry -> (PAGE_GET_PHYSICAL_ADDR(pd_entry)) -> ptable
//					 ptable -> (PAGE_TABLE_INDEX(v_addr)) -> pt_entry -> (PAGE_GET_PHYSICAL_ADDR(pt_entry)) -> physical address

// definitions usage can be replaced by below functions

// INTERFACE

// initializes the virtual memory manager given the dummy directory created by the initializer 
error_t virt_mem_init(pdirectory* old_directory);

// maps the virtual address given to the physical address given
error_t virt_mem_map_page(pdirectory* dir, physical_addr phys, virtual_addr virt, uint32_t flags);

// unmaps the virtual address given from its associated physical address
error_t virt_mem_unmap_page(pdirectory* dir, virtual_addr virt);

// allocates a virtual page with the default flags
error_t virt_mem_alloc_page(virtual_addr base);

// allocates a virtual page with flags
error_t virt_mem_alloc_page_f(virtual_addr base, uint32_t flags);

// frees a virtual page
error_t virt_mem_free_page(virtual_addr base);

// switch page directory
error_t virt_mem_switch_directory(physical_addr pdbr);

// copies all the entries of a directory and returns the physical address of the new one
pdirectory* virt_mem_deep_clone_directory(pdirectory* dir);

// get the current page directory
pdirectory* virt_mem_get_directory();

// flush a cached virtual address
void virt_mem_flush_TLB_entry(virtual_addr addr);

// clear a page table
error_t virt_mem_ptable_clear(ptable* table);

// returns entry of ptable p based on addr
pt_entry* virt_mem_ptable_lookup_entry(ptable* p, virtual_addr addr);

// clear a page directory
error_t virt_mem_pdirectory_clear(pdirectory* pdir);

// returns entry of pdirectory p based on addr
pd_entry* virt_mem_pdirectory_lookup_entry(pdirectory* p, virtual_addr addr);

// print a directory structure (for debug purposes)
void virt_mem_print(pdirectory* dir);

// returns the physical address associated with this virtual address
physical_addr virt_mem_get_phys_addr(virtual_addr addr);

// returns true if the page given by the virtual address is present IN RAM
int virt_mem_is_page_present(virtual_addr addr);

// creates a page table for the dir address space
// error_t virt_mem_create_table(pdirectory* dir, virtual_addr addr, uint32_t flags);

// creates a new address space and returns its physical address
physical_addr virt_mem_create_address_space();

// maps the kernel pages to the directory given
error_t virt_mem_map_kernel_space(pdirectory* pdir);

error_t virt_mem_switch_to_kernel_directory();

// return the page size
uint32_t virt_mem_get_page_size();

// returns the number of the page tables that are currently marked as present
uint32_t virt_mem_count_present_tables(pdirectory* pdir);

// returns the current directory based on recursive mapping
pdirectory* virt_mem_get_current_directory();

// returns the address of the page table requested by index based on recursive mapping
// index must be < 1023. When index is 1023 the last table is used which returns the current page directory
ptable* virt_mem_get_page_table(uint32_t index);

// returns the page table index that is associated with this address
ptable* virt_mem_get_page_table_index_by_address(virtual_addr addr);


#endif