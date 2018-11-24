#include "vmmngr_pte.h"

// INTERFACE FUNCTIONS

void pt_entry_add_attrib(pt_entry* entry, uint32_t attrib)
{
	*entry |= attrib;
}

void pt_entry_del_attrib(pt_entry* entry, uint32_t attrib)
{
	*entry &= ~attrib;
}

int pt_entry_test_attrib(pt_entry* entry, uint32_t attrib)
{
	return ((*entry & attrib) == attrib);
}

void pt_entry_set_frame(pt_entry* entry, physical_addr_t addr)
{
	*entry = (*entry & ~I86_PTE_FRAME) | addr;		// addr is 4KB aligned => erase frame bits, set them with addr
}

int pt_entry_is_present(pt_entry entry)
{
	return entry & I86_PTE_PRESENT;
}

int pt_entry_is_writable(pt_entry entry)
{
	return entry & I86_PTE_WRITABLE;
}

physical_addr_t pt_entry_get_frame(pt_entry entry)
{
	return entry & I86_PTE_FRAME;
}
