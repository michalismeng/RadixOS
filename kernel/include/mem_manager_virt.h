#ifndef MMNGR_VIRTUAL_H_130516
#define MMNGR_VIRTUAL_H_130516

#include <types.h>
#include <isr.h>

#include <vmmngr_pte.h>
#include <vmmngr_pde.h>

enum error_t
{
	VMEM_NONE,
	VMEM_BAD_ARGUMENT,
	ERROR_OK = 1,
	ERROR_OCCUR = 0
};

#define PAGES_PER_TABLE 1024	// intel arch definitions
#define TABLES_PER_DIR	1024
#define PAGE_SIZE 4096

#define DEFAULT_FLAGS I86_PDE_PRESENT | I86_PDE_WRITABLE	// default flags for page tables and pages.

// definitions for entry extraction based on virtual address. (see virtual address format)

#define PAGE_DIR_INDEX(x)			( ((x) >> 22) & 0x3ff )		// Get the 10 upper bits of x
#define PAGE_TABLE_INDEX(x)			( ((x) >> 12) & 0x3ff )		// Get the 10 "middle" bits of x
#define PAGE_GET_PHYSICAL_ADDR(x)	( (*x) & ~0xfff )			// Physical address is 4KB aligned, so return all bits except the 12 first

void page_fault(iregisters_t* regs);
// page table definition
typedef struct page_table
{
	pt_entry entries[PAGES_PER_TABLE];
} ptable;

// page directory definition
typedef struct page_directory
{
	pd_entry entries[TABLES_PER_DIR];
} pdirectory;

// USEFUL DATA CHAIN cr3 -> pdirectory -> (PAGE_DIR_INDEX(v_addr)) -> pd_entry -> (PAGE_GET_PHYSICAL_ADDR(pd_entry)) -> ptable
//					 ptable -> (PAGE_TABLE_INDEX(v_addr)) -> pt_entry -> (PAGE_GET_PHYSICAL_ADDR(pt_entry)) -> physical address

// definitions usage can be replaced by below functions

// INTERFACE

// maps the virtual address given to the physical address given
error_t vmmngr_map_page(pdirectory* dir, physical_addr phys, virtual_addr virt, uint32_t flags);

// initializes the virtual memory manager
error_t vmmngr_initialize(uint32_t kernel_pages);

// allocates a virtual page with the default flags
error_t vmmngr_alloc_page(virtual_addr base);

// allocates a virtual page with flags
error_t vmmngr_alloc_page_f(virtual_addr base, uint32_t flags);

// frees a virtual page
error_t vmmngr_free_page(pt_entry* entry);

// frees a virtual page using a virtual address
void vmmngr_free_page_addr(virtual_addr addr);

// switch page directory
error_t vmmngr_switch_directory(pdirectory* dir, physical_addr pdbr);

// get the current page directory
pdirectory* vmmngr_get_directory();

// flush a cached virtual address
void vmmngr_flush_TLB_entry(virtual_addr addr);

// clear a page table
error_t vmmngr_ptable_clear(ptable* table);

// returns entry of ptable p based on addr
pt_entry* vmmngr_ptable_lookup_entry(ptable* p, virtual_addr addr);

// clear a page directory
error_t vmmngr_pdirectory_clear(pdirectory* pdir);

// returns entry of pdirectory p based on addr
pd_entry* vmmngr_pdirectory_lookup_entry(pdirectory* p, virtual_addr addr);

// print a directory structure (for debug purposes)
void vmmngr_print(pdirectory* dir);

// returns the physical address associated with this virtual address
physical_addr vmmngr_get_phys_addr(virtual_addr addr);

// returns true if the page given by the virtual address is present IN RAM
int vmmngr_is_page_present(virtual_addr addr);

// creates a page table for the dir address space
// error_t vmmngr_create_table(pdirectory* dir, virtual_addr addr, uint32_t flags);

// creates a new address space
pdirectory* vmmngr_create_address_space();

// maps the kernel pages to the directory given
error_t vmmngr_map_kernel_space(pdirectory* pdir);

error_t vmmngr_switch_to_kernel_directory();

// return the page size
uint32_t vmmngr_get_page_size();

#endif