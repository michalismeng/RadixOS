#ifndef MMNGR_VIRTUAL_H_130516
#define MMNGR_VIRTUAL_H_130516

#include <types.h>
#include <mem_manager_virt_aux.h>
#include <isr.h>

// TODO: Move these to some virtual memory definitions...
// page table definition


#define PAGE_SIZE 4096

#define VIRT_MEM_DEFAULT_PDE_FLAGS I86_PDE_PRESENT | I86_PDE_WRITABLE	// default flags for page tables.
#define VIRT_MEM_DEFAULT_PTE_FLAGS I86_PTE_PRESENT | I86_PTE_WRITABLE	// default flags for page entries.

// USEFUL DATA CHAIN cr3 -> pdirectory -> (PAGE_DIR_INDEX(v_addr)) -> pd_entry -> (PAGE_GET_physical_addr_t(pd_entry)) -> ptable
//					 ptable -> (PAGE_TABLE_INDEX(v_addr)) -> pt_entry -> (PAGE_GET_physical_addr_t(pt_entry)) -> physical address

// INTERFACE

// initializes the virtual memory manager given the dummy directory created by the initializer 
physical_addr_t virt_mem_init(pdirectory_t* old_directory);

// maps the virtual address given to the physical address given
error_t virt_mem_map_page(vrec_table_t base, physical_addr_t phys, virtual_addr_t virt, uint32_t page_flags);

// unmaps the virtual address given from its associated physical address
error_t virt_mem_unmap_page(vrec_table_t base, virtual_addr_t virt);

// allocates a virtual page with the default flags
error_t virt_mem_alloc_page(virtual_addr_t addr);

// allocates a virtual page with flags
error_t virt_mem_alloc_page_f(virtual_addr_t addr, uint32_t flags);

// frees a virtual page
error_t virt_mem_free_page(virtual_addr_t addr);

// switch page directory
error_t virt_mem_switch_directory(physical_addr_t pdbr);

// copies all the entries of a directory and returns the physical address of the new one
physical_addr_t virt_mem_deep_clone_directory(pdirectory_t* dir);

// flush a cached virtual address
void virt_mem_flush_TLB_entry(virtual_addr_t addr);

// clear a page table
error_t virt_mem_ptable_clear(ptable_t* table);

// clear a page directory
error_t virt_mem_pdirectory_clear(pdirectory_t* pdir);

// print a directory structure (for debug purposes)
void virt_mem_print(vrec_table_t base);

// returns the physical address associated with this virtual address
physical_addr_t virt_mem_get_phys_addr(virtual_addr_t addr);

// returns true if the page given by the virtual address is marked as present
int virt_mem_is_page_present(virtual_addr_t addr);

// creates a new address space and returns its physical address
physical_addr_t virt_mem_create_address_space();

// maps the kernel pages to the directory given
// error_t virt_mem_map_kernel_space(pdirectory* pdir);

// return the page size
uint32_t virt_mem_get_page_size();

// returns the number of the page tables that are currently marked as present
uint32_t virt_mem_count_present_tables(pdirectory_t* pdir);

error_t virt_mem_set_foreign_address_space(vrec_table_t current, vrec_table_t space, physical_addr_t pdir);

void virt_mem_set_recursive_table(vrec_table_t current, vrec_table_t target, physical_addr_t pdir);

#endif