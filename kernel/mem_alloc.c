#include <mem_alloc.h>
#include <kernel_definitions.h>
#include <mem_manager_virt.h>

static virtual_addr_t current = 0;

virtual_addr_t alloc_perm()
{
	if(current == 0)		// lame initialization to first free virtual address after the kernel
		current = (ceil_division(KERNEL_END, virt_mem_get_page_size())) * virt_mem_get_page_size();

	virtual_addr_t temp = current;
	current += virt_mem_get_page_size();

	if(virt_mem_alloc_page(temp) != ERROR_OK)
		PANIC("Cannot allocate kernel memory");
	return temp;
}