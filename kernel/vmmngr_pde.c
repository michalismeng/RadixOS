#include "vmmngr_pde.h"

// INTERFACE FUNCTIONS

void pd_entry_add_attrib(pd_entry* e, uint32_t attrib) 
{
	*e |= attrib;
}

void pd_entry_del_attrib(pd_entry* e, uint32_t attrib)
{
	*e &= ~attrib;
}

int pd_entry_test_attrib(pd_entry* e, uint32_t attrib)
{
	return ((*e & attrib) == attrib);
}

void pd_entry_set_frame(pd_entry* e, physical_addr addr)
{
	*e = (*e & ~I86_PDE_FRAME) | addr;
}

int pd_entry_is_present(pd_entry e)
{
	return e & I86_PDE_PRESENT;
}

int pd_entry_is_writable(pd_entry e)
{
	return e & I86_PDE_WRITABLE;
}

int pd_entry_is_user(pd_entry e) 
{
	return e & I86_PDE_USER;
}

int pd_entry_is_4mb(pd_entry e) 
{
	return e & I86_PDE_4MB;
}

struct page_table* pd_entry_get_frame(pd_entry e)
{
	return e & I86_PDE_FRAME;
}