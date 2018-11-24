#include <mem_manager_virt_aux.h>

vrec_table_t virt_mem_get_self_recursive_table()
{
	return 0xC0000000;
}

vrec_table_t virt_mem_get_foreign_recursive_table()
{
	return 0xC0400000;
}

ptable_t* virt_mem_get_page_table(vrec_table_t base, virtual_addr_t address)
{
    uint32_t index = virt_mem_get_page_table_index(address);
	return (ptable_t*)(base + (index << 12));
}

pdirectory_t* virt_mem_get_directory(vrec_table_t base)
{
	return (pdirectory_t*)virt_mem_get_page_table(base, base);
}

pdirectory_t* virt_mem_get_current_directory()
{
	return (pdirectory_t*)virt_mem_get_page_table(virt_mem_get_self_recursive_table(), virt_mem_get_self_recursive_table());
}

pdirectory_t* virt_mem_get_foreign_directory()
{
	return (pdirectory_t*)virt_mem_get_page_table(virt_mem_get_self_recursive_table(), virt_mem_get_foreign_recursive_table());
}

pd_entry* virt_mem_get_page_directory_entry(pdirectory_t* p, virtual_addr_t addr)
{
    if (p)
		return &p->entries[virt_mem_get_page_table_index(addr)];

	return 0;
}

pt_entry* virt_mem_get_page_table_entry(ptable_t* p, virtual_addr_t addr)
{
	if (p)
		return &p->entries[virt_mem_get_page_index(addr)];

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

pd_entry* virt_mem_get_page_directory_entry_by_addr(vrec_table_t base, virtual_addr_t addr)
{
	pdirectory_t* directory = virt_mem_get_directory(base);
	return virt_mem_get_page_directory_entry(directory, addr);
}

pt_entry* virt_mem_get_page_table_entry_by_addr(vrec_table_t base, virtual_addr_t addr)
{
	ptable_t* table = virt_mem_get_page_table(base, addr);
	return virt_mem_get_page_table_entry(table, addr);
}