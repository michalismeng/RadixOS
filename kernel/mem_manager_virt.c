#include <mem_manager_virt.h>
#include <mem_manager_phys.h>
#include <debug.h>
#include <utility.h>
#include <iregs.h>
#include <gst.h>

// private data

// These fields should be processor specific
pdirectory*	current_directory = 0;		// current page directory
physical_addr current_pdbr = 0;			// current page directory base register

pdirectory* kernel_directory = 0;		// kernel page directory
// ----------------------------------------------------------------------------------

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

#pragma region comment
// uint32_t page_fault_calculate_present_flags(uint32_t area_flags)
// {
// 	uint32_t flags = I86_PDE_PRESENT;

// 	if (CHK_BIT(area_flags, MMAP_WRITE))
// 		flags |= I86_PDE_WRITABLE;

// 	if (CHK_BIT(area_flags, MMAP_USER))
// 		flags |= I86_PDE_USER;

// 	return flags;
// }

// void page_fault_alloc_page(uint32_t area_flags, virtual_addr address)
// {
// 	uint32_t flags = page_fault_calculate_present_flags(area_flags);

// 	if (CHK_BIT(area_flags, MMAP_IDENTITY_MAP))
// 		vmmngr_map_page(vmmngr_get_directory(), address, address, flags);
// 	else
// 		vmmngr_alloc_page_f(address, flags);
// }

// void page_fault_bottom(thread_exception te)
// {
// 	thread_exception_print(&te);
// 	uint32_t& addr = te.data[0];
// 	uint32_t& code = te.data[1];

// 	serial_printf("PAGE_FALUT: PROC: %u ADDRESS: %h, THREAD: %u, CODE: %h\n", process_get_current()->id, addr, thread_get_current()->id, code);

// 	if (process_get_current()->contract_spinlock == 1)
// 		PANIC("PAge fault spinlock is already reserved\n");

// 	spinlock_acquire(&process_get_current()->contract_spinlock);
// 	vm_area* p_area = vm_contract_find_area(&thread_get_current()->parent->memory_contract, addr);

// 	if (p_area == 0)
// 	{
// 		serial_printf("could not find address %h in memory contract", addr);

// 		page_fault_alloc_page(MMAP_WRITE | MMAP_ANONYMOUS | MMAP_PRIVATE, addr);
// 		spinlock_release(&process_get_current()->contract_spinlock);
// 		return;

// 		PANIC("");		// terminate thread and process with SIGSEGV
// 	}

// 	vm_area area = *p_area;
// 	spinlock_release(&process_get_current()->contract_spinlock);

// 	// tried to acccess inaccessible page
// 	if ((area.flags & MMAP_PROTECTION) == MMAP_NO_ACCESS)
// 	{
// 		serial_printf("address: %h is inaccessible\n", addr);
// 		PANIC("");
// 	}

// 	// tried to write to read-only or inaccessible page
// 	if (page_fault_error_is_write(code) && (area.flags & MMAP_WRITE) != MMAP_WRITE)
// 	{
// 		serial_printf("cannot write to address: %h\n", addr);
// 		PANIC("");
// 	}

// 	// tried to read a write-only or inaccesible page ???what???
// 	/*if (!page_fault_error_is_write(code) && CHK_BIT(area.flags, MMAP_READ))
// 	{
// 		serial_printf("cannot read from address: %h", addr);
// 		PANIC("");
// 	}*/

// 	// if the page is present then a violation happened (we do not implement swap out/shared anonymous yet)
// 	if (page_fault_error_is_page_present(code) == true)
// 	{
// 		serial_printf("memory violation at address: %h with code: %h\n", addr, code);
// 		serial_printf("area flags: %h\n", area.flags);
// 		PANIC("");
// 	}

// 	// here we found out that the page is not present, so we need to allocate it properly
// 	if (CHK_BIT(area.flags, MMAP_PRIVATE))
// 	{
// 		if (CHK_BIT(area.flags, MMAP_ALLOC_IMMEDIATE))
// 		{
// 			// loop through all addresses and map them
// 			for (virtual_addr address = area.start_addr; address < area.end_addr; address += 4096)
// 				//if (CHK_BIT(area.flags, MMAP_ANONYMOUS))	ALLOC_IMMEDIATE works only for anonymous (imposed in mmap)
// 				page_fault_alloc_page(area.flags, address);
// 		}
// 		else
// 		{
// 			if (CHK_BIT(area.flags, MMAP_ANONYMOUS))
// 				page_fault_alloc_page(area.flags, addr & (~0xFFF));
// 			else
// 			{
// 				uint32_t flags = page_fault_calculate_present_flags(area.flags);
// 				vmmngr_alloc_page_f(addr & (~0xFFF), flags);

// 				uint32_t read_start = area.offset + ((addr - area.start_addr) / PAGE_SIZE) * PAGE_SIZE;		// file read start
// 				uint32_t read_size = PAGE_SIZE;		// we read one page at a time (not the whole area as this may not be necessary)

// 				//if (read_start < area.start_addr + PAGE_SIZE)	// we are reading the first page so subtract offset from read_size
// 				//	read_size -= area.offset;

// 				serial_printf("gfd: %u, reading at mem: %h, phys: %h file: %h, size: %u\n", area.fd, addr & (~0xfff), vmmngr_get_phys_addr(addr & (~0xfff)),
// 					read_start, read_size);

// 				gfe* entry = gft_get(area.fd);
// 				if (entry == 0)
// 				{
// 					serial_printf("area.fd = %u", area.fd);
// 					PANIC("page fault gfd entry = 0");
// 				}

// 				// read one page from the file offset given at the 4KB-aligned fault address 
// 				if (read_file_global(area.fd, read_start, read_size, addr & (~0xFFF), VFS_CAP_READ | VFS_CAP_CACHE) != read_size)
// 				{
// 					serial_printf("read fd: %u\n", area.fd);
// 					PANIC("mmap anonymous file read less bytes than expected");
// 				}
// 			}
// 		}
// 	}
// 	else		// MMAP_SHARED
// 	{
// 		if (CHK_BIT(area.flags, MMAP_ANONYMOUS))
// 			PANIC("A shared area cannot be marked as anonymous yet.");
// 		else
// 		{
// 			// in the shared file mapping the address to read is ignored as data are read only to page cache. 
// 			uint32_t read_start = area.offset + ((addr & (~0xfff)) - area.start_addr);
// 			gfe* entry = gft_get(area.fd);

// 			if (read_file_global(area.fd, read_start, PAGE_SIZE, -1, VFS_CAP_READ | VFS_CAP_CACHE) != PAGE_SIZE)
// 				PANIC("mmap shared file failed");

// 			virtual_addr used_cache = page_cache_get_buffer(area.fd, read_start / PAGE_SIZE);
// 			//serial_printf("m%h\n", used_cache);

// 			uint32_t flags = page_fault_calculate_present_flags(area.flags);
// 			vmmngr_map_page(vmmngr_get_directory(), vmmngr_get_phys_addr(used_cache), addr & (~0xfff), flags/*DEFAULT_FLAGS*/);
// 			//serial_printf("shared mapping fd: %u, cache: %h, phys cache: %h, read: %u, addr: %h\n", area.fd, used_cache, used_cache, read_start, addr);
// 		}
// 	}
// }
#pragma endregion

void page_fault_handler(iregisters_t* regs)
{
	uint32_t cr2;
    __asm__ __volatile__ (
        "mov %%cr2, %%eax\n\t"
        "mov %%eax, %0\n\t"
		: "=m" (cr2)
    	: /* no input */
    	: "%eax");

	printfln("page fault occured at: %h", cr2);
	virt_mem_map_page(virt_mem_get_current_directory(), cr2, cr2, VIRT_MEM_DEFAULT_PTE_FLAGS);
}

// creates a page table for the dir address space
error_t virt_mem_create_table(pdirectory* dir, virtual_addr addr, uint32_t flags)
{
	pd_entry* entry = virt_mem_pdirectory_lookup_entry(dir, addr);
	if (!entry)
		return ERROR_OCCUR;

	ptable* table = (ptable*)phys_mem_alloc_above_1mb();
	if (!table)
		return ERROR_OCCUR;		// not enough memory!!

	memset(table, 0, sizeof(ptable));
	pd_entry_set_frame(entry, (physical_addr)table);
	pd_entry_add_attrib(entry, flags);

	return ERROR_OK;
}

// public functions

physical_addr virt_mem_init(pdirectory* old_directory)
{
	physical_addr new_dir_addr = virt_mem_deep_clone_directory(old_directory);
	if(new_dir_addr == 0)
		return 0;

	pdirectory* new_dir = (pdirectory*)new_dir_addr;

	// map the directory entry of address 0xC0000000 to point to itself => recursion

	uint32_t index = virt_mem_get_directory_index_by_address(0xC0000000);
	printfln("index of recursion page table: %u", index);
	pd_entry_add_attrib(&new_dir->entries[index], VIRT_MEM_DEFAULT_PDE_FLAGS);
	pd_entry_set_frame(&new_dir->entries[index], new_dir);

	virt_mem_switch_directory(new_dir_addr);
	isr_register(14, page_fault_handler);

	return new_dir_addr;
}

error_t virt_mem_map_page(pdirectory* dir, physical_addr phys, virtual_addr virt, uint32_t flags)
{
	// our goal is to get the pt_entry indicated by virt and set its frame to phys.

	pd_entry* e = virt_mem_pdirectory_lookup_entry(dir, virt);

	if (!pd_entry_is_present(e))								// table is not present
		if (virt_mem_create_table(dir, virt, flags) != ERROR_OK)
			return ERROR_OCCUR;

	// here we have a guaranteed working table (perhaps empty)

	ptable* table = pd_entry_get_frame(*e);
	pt_entry* page = virt_mem_ptable_lookup_entry(table, virt);	// we have the page
	if (page == 0)
		return ERROR_OCCUR;
	
	*page = 0;												// remove previous flags
	*page |= flags;											// and set the new ones
	pt_entry_set_frame(page, phys);

	// TODO: This must happen here only when dir is the current page directory
	//virt_mem_flush_TLB_entry(virt);							// TODO: inform the other cores of the mapping change

	return ERROR_OK;
}

error_t virt_mem_unmap_page(pdirectory* dir, virtual_addr virt)
{
	pd_entry* e = virt_mem_pdirectory_lookup_entry(dir, virt);
	ptable* table = pd_entry_get_frame(*e);
	pt_entry* page = virt_mem_ptable_lookup_entry(table, virt);

	*page = 0;

	return ERROR_OK;
}

error_t virt_mem_alloc_page(virtual_addr base)
{
	return virt_mem_alloc_page_f(base, VIRT_MEM_DEFAULT_PTE_FLAGS);
}

error_t virt_mem_alloc_page_f(virtual_addr base, uint32_t flags)
{
	// assume that 'base' is not already allocated

	physical_addr addr = (physical_addr)phys_mem_alloc_above_1mb();
	if (addr == 0)
		return ERROR_OCCUR;

	if (virt_mem_map_page(virt_mem_get_directory(), addr, base, flags) != ERROR_OK)
		return ERROR_OCCUR;

	// TODO: TLB shootdown

	return ERROR_OK;
}

error_t virt_mem_free_page(virtual_addr base)
{
	physical_addr addr = virt_mem_get_phys_addr(base);
	if(addr == 0)
		return ERROR_OCCUR;
	
	if(virt_mem_unmap_page(virt_mem_get_directory(), base) != ERROR_OK)
		return ERROR_OCCUR;

	phys_mem_dealloc(addr);		// TODO: Add error checking

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

physical_addr virt_mem_deep_clone_directory(pdirectory* dir)
{
	pdirectory* new_dir = (pdirectory*)phys_mem_alloc_above_1mb();
	memcpy(new_dir, dir, PAGE_SIZE);

	for(int i = 0; i < VIRT_MEM_PAGES_PER_DIR; i++)
	{
		pd_entry* new_e = &new_dir->entries[i];

		// if the table is present
		if(pd_entry_is_present(*new_e))
		{
			ptable* new_table = (ptable*)phys_mem_alloc_above_1mb();	// allocate new space for the table entries
			ptable* old_table = pd_entry_get_frame(*new_e);				// the page table in the old directory (we used the directory's frame pointer)
			memcpy(new_table, old_table, PAGE_SIZE);

			pd_entry_set_frame(new_e, new_table);						// set the new table frame for our new directory entry
		}
	}

	return new_dir;
}

pdirectory* virt_mem_get_directory()
{
	return current_directory;
}

void virt_mem_flush_TLB_entry(virtual_addr addr)
{
	asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
}

error_t virt_mem_ptable_clear(ptable* table)
{
	memset(table, 0, sizeof(ptable));
	phys_mem_dealloc((physical_addr)table);

	return ERROR_OK;
}

error_t virt_mem_pdirectory_clear(pdirectory* pdir)
{
	for (int i = 0; i < VIRT_MEM_PAGES_PER_DIR; i++)
	{
		if (virt_mem_ptable_clear((ptable*)pd_entry_get_frame(pdir->entries[i])) != ERROR_OK)
			return ERROR_OCCUR;
	}

	memset(pdir, 0, sizeof(pdirectory));
	phys_mem_dealloc((physical_addr)pdir);		// TODO: This should be physical address, not virtual

	return ERROR_OK;
}

pt_entry* virt_mem_ptable_lookup_entry(ptable* p, virtual_addr addr)
{
	if (p)
		return &p->entries[PAGE_TABLE_INDEX(addr)];

	// set_last_error(EINVAL, VMEM_BAD_ARGUMENT, EO_virt_mem);
	return 0;
}

pd_entry* virt_mem_pdirectory_lookup_entry(pdirectory* p, virtual_addr addr)
{
	if (p)
		return &p->entries[PAGE_DIR_INDEX(addr)];

	// set_last_error(EINVAL, VMEM_BAD_ARGUMENT, EO_virt_mem);
	return 0;
}

void virt_mem_print(pdirectory* dir)
{
	for (int i = 0; i < 1024; i++)
	{
		if (!pd_entry_test_attrib(&dir->entries[i], I86_PDE_PRESENT))
			continue;

		ptable* table = (ptable*)PAGE_GET_PHYSICAL_ADDR(&dir->entries[i]);

		printfln("table %i is present at %h", i, table);

		for (int j = 0; j < 1024; j++)
		{
			if (!pt_entry_test_attrib(&table->entries[j], I86_PTE_PRESENT))
				continue;

			printf("page %i is present with frame: %h ", j, pt_entry_get_frame(table->entries[j]));
		}
	}
}

// checked
physical_addr virt_mem_get_phys_addr(virtual_addr addr)
{
	pd_entry* e = virt_mem_pdirectory_lookup_entry(virt_mem_get_directory(), addr);
	if (!e)
		return 0;

	ptable* table = pd_entry_get_frame(*e);
	pt_entry* page = virt_mem_ptable_lookup_entry(table, addr);
	if (!page)
		return 0;

	physical_addr p_addr = pt_entry_get_frame(*page);

	p_addr += (addr & 0xfff);		// add in-page offset
	return p_addr;
}

// checked
int virt_mem_is_page_present(virtual_addr addr)
{
	pd_entry* e = virt_mem_pdirectory_lookup_entry(virt_mem_get_directory(), addr);
	if (e == 0 || !pd_entry_is_present(*e))
		return 0;

	ptable* table = (ptable*)pd_entry_get_frame(*e);
	if (table == 0)
		return 0;

	pt_entry* page = virt_mem_ptable_lookup_entry(table, addr);
	if (page == 0 || !pt_entry_is_present(*page) == 0)
		return 0;

	return 1;
}

// checked
physical_addr virt_mem_create_address_space()
{
	pdirectory* dir = (pdirectory*)phys_mem_alloc_above_1mb();
	if (!dir)
		return 0;

	memset(dir, 0, sizeof(pdirectory));
	return (physical_addr)dir;
}

// performs shallow copy
error_t virt_mem_map_kernel_space(pdirectory* pdir)
{
	if (!pdir)
		return ERROR_OCCUR;

	memcpy(pdir, kernel_directory, sizeof(pdirectory));
	return ERROR_OK;
}

// not necessary
error_t virt_mem_switch_to_kernel_directory()
{
	// return virt_mem_switch_directory(kernel_directory, (physical_addr)kernel_directory);
}

uint32_t virt_mem_get_page_size()
{
	return PAGE_SIZE;
}

uint32_t virt_mem_count_present_tables(pdirectory* pdir)
{
	uint32_t count = 0;

	for(int i = 0; i < VIRT_MEM_PAGES_PER_DIR; i++)
		if(pd_entry_is_present(pdir->entries[i]))
			count++;

	return count;
}

pdirectory* virt_mem_get_current_directory()
{
	uint32_t index = virt_mem_get_directory_index_by_address(0xC0000000);
	return (pdirectory*)virt_mem_get_page_table(index);
}

ptable* virt_mem_get_page_table(uint32_t index)
{
	return (ptable*)(0xC0000000 + (index << 12));
}

uint32_t virt_mem_get_page_table_index_by_address(virtual_addr addr)
{
	return (addr & 0x003FF000) >> 12;			// take the middle 10 bits as tha page table index
}

uint32_t virt_mem_get_directory_index_by_address(virtual_addr addr)
{
	return (addr & 0xFFC00000) >> 22;
}