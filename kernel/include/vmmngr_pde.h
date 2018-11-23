#ifndef VMMNGR_PDE_H_160516
#define VMMNGR_PDE_H_160516

#include <types.h>

// define utilities for a page directory entry (pde)

struct page_table;

enum PAGE_PDE_FLAGS
{
	I86_PDE_PRESENT = 1,
	I86_PDE_WRITABLE = 2,
	I86_PDE_USER = 4,
	I86_PDE_PWT = 8,
	I86_PDE_PCD = 0x10,
	I86_PDE_ACCESSED = 0x20,
	I86_PDE_DIRTY = 0x40,
	I86_PDE_4MB = 0x80,
	I86_PDE_CPU_GLOBAL = 0x100,
	I86_PDE_LV4_GLOBAL = 0x200,
	I86_PDE_FRAME = 0xFFFFF000		// here the first digit was 7 instead of F (0x7FFF) by brokenthorn...
};

typedef uint32_t pd_entry;

// INTERFACE

// sets a flag in the page table entry
void pd_entry_add_attrib(pd_entry* entry, uint32_t attrib);

// clears a flag in the page table entry
void pd_entry_del_attrib(pd_entry* entry, uint32_t attrib);

// checks if the given attribute is set
int pd_entry_test_attrib(pd_entry* entry, uint32_t attrib);

// sets a frame to page table
void pd_entry_set_frame(pd_entry* entry, physical_addr addr);

// test if page is present
int pd_entry_is_present(pd_entry entry);

// test if directory is user mode
int pd_entry_is_user(pd_entry entry);

// test if directory contains 4mb pages
int pd_entry_is_4mb(pd_entry entry);

// test if page is writable
int pd_entry_is_writable(pd_entry entry);

// get page table entry frame address
struct page_table* pd_entry_get_frame(pd_entry entry);

#endif