#include <mem_manager_virt.h>
#include <mem_manager_phys.h>
#include <debug.h>
#include <utility.h>
#include <gst.h>

// private data

// when fault occured the page was present in memory
int page_fault_error_is_page_present(uint32_t error)
{
	return (error & 0x1);
}

// the fault occured due to a write attempt
int page_fault_error_is_write(uint32_t error)
{
	return (error & 0x2);
}

// the fault occured while the cpu was in CPL=3
int page_fault_error_is_user(uint32_t error)
{
	return (error & 0x4);
}

int32_t page_fault_handler(trap_frame_t* regs)
{
	uint32_t cr2;

    asm volatile (
        "mov %%cr2, %%eax\n\t"
        "mov %%eax, %0\n\t"
		: "=m" (cr2)
    	: /* no input */
    	: "%eax");

	printfln("page fault occured at: %h, eip: %h", cr2, regs->eip);
	printf("reason: page ");
	printf(regs->err_code & 1 ? "present " : "not present ");
	printf(regs->err_code & 2 ? "write " : "read ");
	printf(regs->err_code & 4 ? "in user " : "in kernel ");
	printf(regs->err_code & 8 ? "for resved bit  " : "");
	printfln(regs->err_code & 16 ? "instr fetch " : "not instr fetch ");

	// printfln("page phys mem: %h", virt_mem_get_phys_addr(cr2));

	printfln("");
	trap_frame_print(regs);

    if(page_fault_error_is_user(regs->err_code))
        PANIC("\nSIGSEGV");


	virt_mem_map_page(virt_mem_get_current_address_space(), cr2, cr2, VIRT_MEM_DEFAULT_PTE_FLAGS);

	return 0;
}

// creates a page table for the dir address space
error_t virt_mem_create_table(address_space_t base, virtual_addr_t addr, uint32_t flags)
{
	pdirectory_t* pdir = virt_mem_get_directory(base);
	pd_entry* entry = virt_mem_get_page_directory_entry(pdir, addr);
	if (!entry)
		return ERROR_OCCUR;

	physical_addr table_phys = phys_mem_alloc_above_1mb();

	if (!table_phys)
		return ERROR_OCCUR;		// not enough memory!!

	pd_entry_set_frame(entry, table_phys);
	pd_entry_add_attrib(entry, flags);

	ptable_t* table = virt_mem_get_page_table(base, addr);
	memset(table, 0, sizeof(ptable_t));

	return ERROR_OK;
}

// public functions

physical_addr virt_mem_init(pdirectory_t* old_directory)
{
	// TODO: Here we assume new_dir_addr is identity mapped so we can directly access it.
	physical_addr new_dir_addr = virt_mem_deep_clone_directory(old_directory);

	if(new_dir_addr == 0)
		return 0;

	pdirectory_t* new_dir = (pdirectory_t*)new_dir_addr;

	// map the directory entry of address 0xC0000000 to point to the page directory structure => recursion
	uint32_t index = virt_mem_get_page_table_index(0xC0000000);
	pd_entry_add_attrib(&new_dir->entries[index], VIRT_MEM_DEFAULT_PDE_FLAGS);
	pd_entry_set_frame(&new_dir->entries[index], new_dir_addr);

	virt_mem_switch_directory(new_dir_addr);
	isr_register(14, page_fault_handler);

	return new_dir_addr;
}

error_t virt_mem_map_page(address_space_t base, physical_addr phys, virtual_addr_t virt, uint32_t flags)
{
	// our goal is to get the pt_entry indicated by virt and set its frame to phys.
	pdirectory_t* pdir = virt_mem_get_directory(base);
	pd_entry* e = virt_mem_get_page_directory_entry(pdir, virt);

	if (!pd_entry_is_present(*e))								// table is not present
		if (virt_mem_create_table(base, virt, flags) != ERROR_OK)
			return ERROR_OCCUR;

	// here we have a guaranteed working table (perhaps empty)

	ptable_t* table = virt_mem_get_page_table(base, virt);
	pt_entry* page = virt_mem_get_page_table_entry(table, virt);	// we have the page

	if (page == 0)
		return ERROR_OCCUR;
	
	// TODO: Fix this
	*page = 0;												// remove previous flags
	pt_entry_set_frame(page, phys);
	pt_entry_add_attrib(page, flags);

	// TODO: This must happen here only when dir is the current page directory
	//virt_mem_flush_TLB_entry(virt);							// TODO: inform the other cores of the mapping change

	return ERROR_OK;
}

error_t virt_mem_unmap_page(address_space_t base, virtual_addr_t virt)
{
	ptable_t* table = virt_mem_get_page_table(base, virt);
	pt_entry* page = virt_mem_get_page_table_entry(table, virt);

	*page = 0;

	return ERROR_OK;
}

error_t virt_mem_alloc_page(virtual_addr_t addr)
{
	return virt_mem_alloc_page_f(addr, VIRT_MEM_DEFAULT_PTE_FLAGS);
}

error_t virt_mem_alloc_page_f(virtual_addr_t addr, uint32_t flags)
{
	// assume that 'base' is not already allocated

	physical_addr phys = phys_mem_alloc_above_1mb();
	if (phys == 0)
		return ERROR_OCCUR;

	if (virt_mem_map_page(virt_mem_get_current_address_space(), phys, addr, flags) != ERROR_OK)
		return ERROR_OCCUR;

	// TODO: TLB shootdown

	return ERROR_OK;
}

error_t virt_mem_free_page(virtual_addr_t addr)
{
	physical_addr phys = virt_mem_get_phys_addr(addr);
	if(phys == 0)
		return ERROR_OCCUR;
	
	if(virt_mem_unmap_page(virt_mem_get_current_address_space(), addr) != ERROR_OK)
		return ERROR_OCCUR;

	if(phys_mem_dealloc(phys) != ERROR_OK)		// TODO: Add error checking
		return ERROR_OCCUR;

	// TODO: TLB shootdown

	return ERROR_OK;
}

error_t virt_mem_switch_directory(physical_addr new_dir)
{
	// if the page directory hasn't change do not flush cr3 as such an action is a performance hit
	// if (pmmngr_get_PDBR() == pdbr)
	// 	return ERROR_OK;				TODO: this must be per-processor

	asm ("movl %0, %%eax; movl %%eax, %%cr3;"::"r"(new_dir):"%eax");

	return ERROR_OK;
}

physical_addr virt_mem_deep_clone_directory(pdirectory_t* dir)
{
	pdirectory_t* new_dir = (pdirectory_t*)phys_mem_alloc_above_1mb();
	memcpy(new_dir, dir, PAGE_SIZE);

	for(int i = 0; i < VIRT_MEM_PAGES_PER_DIR; i++)
	{
		pd_entry* new_e = &new_dir->entries[i];

		// if the table is present
		if(pd_entry_is_present(*new_e))
		{
			ptable_t* new_table = (ptable_t*)phys_mem_alloc_above_1mb();	// allocate new space for the table entries
			ptable_t* old_table = pd_entry_get_frame(*new_e);				// the page table in the old directory (we used the directory's frame pointer)
			memcpy(new_table, old_table, PAGE_SIZE);

			pd_entry_set_frame(new_e, (physical_addr)new_table);						// set the new table frame for our new directory entry
		}
	}

	return (physical_addr)new_dir;
}

void virt_mem_flush_TLB_entry(virtual_addr_t addr)
{
	asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
}

// TODO: This function needs fixing
error_t virt_mem_ptable_clear(ptable_t* table)
{
	memset(table, 0, sizeof(ptable_t));
	phys_mem_dealloc((physical_addr)table);

	return ERROR_OK;
}

// TODO: This function needs fixing
error_t virt_mem_pdirectory_clear(pdirectory_t* pdir)
{
	for (int i = 0; i < VIRT_MEM_PAGES_PER_DIR; i++)
	{
		if (virt_mem_ptable_clear((ptable_t*)pd_entry_get_frame(pdir->entries[i])) != ERROR_OK)
			return ERROR_OCCUR;
	}

	memset(pdir, 0, sizeof(pdirectory_t));
	phys_mem_dealloc((physical_addr)pdir);		// TODO: This should be physical address, not virtual

	return ERROR_OK;
}

void virt_mem_print(address_space_t base)
{
	// pdirectory_t* dir = virt_mem_get_directory(base);
	// for (int i = 0; i < 1024; i++)
	// {
	// 	if (!pd_entry_test_attrib(&dir->entries[i], I86_PDE_PRESENT))
	// 		continue;

	// 	ptable_t* table = virt_mem_get_page_table(base, i);
	// 	physical_addr taddr = pd_entry_get_frame(dir->entries[i]);

	// 	printfln("table %i is present at %h / %h", i, taddr, table);

	// 	for (int j = 0; j < 1024; j++)
	// 	{
	// 		if (!pt_entry_test_attrib(&table->entries[j], I86_PTE_PRESENT))
	// 			continue;

	// 		printfln("page %i is present with frame: %h ", j, pt_entry_get_frame(table->entries[j]));
	// 	}
	// }
}

physical_addr virt_mem_get_phys_addr(virtual_addr_t addr)
{
	pd_entry* e = virt_mem_get_page_directory_entry(virt_mem_get_current_directory(), addr);
	if (!e)
		return 0;

	ptable_t* table = virt_mem_get_page_table(virt_mem_get_current_address_space(), addr);
	pt_entry* page = virt_mem_get_page_table_entry(table, addr);
	if (!page)
		return 0;

	physical_addr p_addr = pt_entry_get_frame(*page);

	p_addr += (addr & 0xfff);		// add in-page offset
	return p_addr;
}

int virt_mem_is_page_present(virtual_addr_t addr)
{
	pd_entry* e = virt_mem_get_page_directory_entry(virt_mem_get_current_directory(), addr);
	if (!e || !pd_entry_is_present(*e))
		return 0;

	ptable_t* table = virt_mem_get_page_table(virt_mem_get_current_address_space(), addr);
	if (!table)
		return 0;

	pt_entry* page = virt_mem_get_page_table_entry(table, addr);
	if (!page || !pt_entry_is_present(*page))
		return 0;

	return 1;
}

physical_addr virt_mem_create_address_space()
{
	pdirectory_t* dir = (pdirectory_t*)phys_mem_alloc_above_1mb();
	if (!dir)
		return 0;

	if(virt_mem_set_foreign_address_space(virt_mem_get_current_address_space(), virt_mem_get_foreign_address_space(), dir) != ERROR_OK)
		return 0;

	pdirectory_t* f_dir = virt_mem_get_foreign_directory();
	memcpy(f_dir, virt_mem_get_directory(virt_mem_get_current_address_space()), sizeof(pdirectory_t));

	// fix the recursion page table entry of the foreign address space to point to itself
	pd_entry* pd = virt_mem_get_page_directory_entry(f_dir, virt_mem_get_current_address_space());
	pd_entry_add_attrib(pd, VIRT_MEM_DEFAULT_PDE_FLAGS);
	pd_entry_set_frame(pd, dir);

	return (physical_addr)dir;
}

// performs shallow copy
// error_t virt_mem_map_kernel_space(pdirectory_t* pdir)
// {
// 	if (!pdir)
// 		return ERROR_OCCUR;

// 	memcpy(pdir, kernel_directory, sizeof(pdirectory_t));
// 	return ERROR_OK;
// }

uint32_t virt_mem_count_present_tables(pdirectory_t* pdir)
{
	uint32_t count = 0;

	for(int i = 0; i < VIRT_MEM_PAGES_PER_DIR; i++)
	{
		if(pd_entry_is_present(pdir->entries[i]))
		{
			printfln("present table %u at: %h", i,  0xC0000000 + (i << 12));
			count++;
		}
	}

	return count;
}

uint32_t virt_mem_get_page_size()
{
	return PAGE_SIZE;
}

error_t virt_mem_set_foreign_address_space(address_space_t current, address_space_t foreign, physical_addr pdir)
{
	pd_entry* pd = virt_mem_get_page_directory_entry_by_addr(current, foreign);
	pd_entry_add_attrib(pd, VIRT_MEM_DEFAULT_PDE_FLAGS);
	pd_entry_set_frame(pd, pdir);

	return ERROR_OK;
}