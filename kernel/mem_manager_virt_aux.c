#include <mem_manager_virt_aux.h>

address_space_t virt_mem_get_current_address_space()
{
	return 0xC0000000;
}

ptable_t* virt_mem_get_page_table(address_space_t base, virtual_addr_t address)
{
    uint32_t index = virt_mem_get_page_table_index(address);
	return (ptable_t*)(base + (index << 12));
}

pdirectory_t* virt_mem_get_directory(address_space_t base)
{
	uint32_t index = virt_mem_get_page_table_index(base);        // we get the directory by getting the virtual address assocaited with the address space
	return (pdirectory_t*)(base + (index << 12));
}

pdirectory_t* virt_mem_get_current_directory()
{
	return virt_mem_get_directory(virt_mem_get_current_address_space());
}

pd_entry* virt_mem_get_page_directory_entry(pdirectory_t* p, virtual_addr_t addr)
{
    if (p)
		return &p->entries[virt_mem_get_page_table_index(addr)];

	// set_last_error(EINVAL, VMEM_BAD_ARGUMENT, EO_virt_mem);
	return 0;
}

pt_entry* virt_mem_get_page_table_entry(ptable_t* p, virtual_addr_t addr)
{
	if (p)
		return &p->entries[virt_mem_get_page_index(addr)];

	// set_last_error(EINVAL, VMEM_BAD_ARGUMENT, EO_virt_mem);
	return 0;
}

uint32_t virt_mem_get_page_index(virtual_addr_t addr)
{
	return (addr & 0x003FF000) >> 12;			// take the middle 10 bits as tha page table index
}

uint32_t virt_mem_get_page_table_index(virtual_addr_t addr)
{
	return (addr & 0xFFC00000) >> 22;			// take the top 10 bits as the directory index 
}