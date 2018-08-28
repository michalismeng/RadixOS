#include <screen.h>
#include <gdt.h>
#include <debug.h>
#include <acpi.h>
#include <pic.h>
#include <multiboot.h>
#include <mem_manager_phys.h>
#include <mem_manager_virt.h>
#include <idt.h>
#include <ioapic.h>
#include <lapic.h>
#include <heap_manager.h>
#include <pit.h>
#include <gst.h>
#include <per_cpu_data.h>
#include <isr.h>
#include <processor_startup.h>
#include <spinlock.h>
#include <kernel_definitions.h>

uint32_t lock = 0;

void kernel_entry(multiboot_info_t* mbd, pdirectory_t* page_dir)
{
	SetForegroundColor(VGA_COLOR_GREEN);
	SetBackgroundColor(VGA_COLOR_BLACK);
	ClearScreen();

	printfln("Hello this is RadixOS");
	printfln("Kernel loaded at: %h", KERNEL_START);
	printfln("Global System Table at: %h", get_gst());

	// initialize the global system table
	memset(get_gst(), 0, sizeof(gst_t));

	// setup a dummy gdt
	gdt_set_gate(GDT_BASE_DUMMY, 0, 0, 0, 0, 0);
	gdt_set_gate(GDT_BASE_DUMMY, 1, 0, 0xFFFFFFFF, GDT_RW | GDT_EX, GDT_SZ | GDT_GRAN);
	gdt_set_gate(GDT_BASE_DUMMY, 2, 0, 0xFFFFFFFF, GDT_RW, GDT_SZ | GDT_GRAN);

	// intstall the dummy gdt
	gdtr_install(GDT_BASE_DUMMY, 3, &get_gst()->gdtr);

	// initialize the idt and isr manager
	idt_init();
	isr_init();

	idtr_install(&get_gst()->idtr);

	// disable the PIC so we can use the io apic
	pic_disable();

	// --------------------------- physical memory manager ---------------------------
	phys_mem_init((mbd->mem_lower + mbd->mem_upper), 0x200000);

	if((mbd->flags & 1) == 0)
		PANIC("Cannot get memory length");
	if((mbd->flags && (1 << 6) == 0))
		PANIC("Cannot get memory map");

	printfln("GRUB detected %u KB lower and %u MB higher", mbd->mem_lower, mbd->mem_upper / 1024);

	multiboot_memory_map_t* entry = (multiboot_memory_map_t*)(mbd->mmap_addr);

	printfln("total memory blocks: %u -> %u KB -> %u MB", phys_mem_get_block_count(), phys_mem_get_block_count() * 4, phys_mem_get_block_count() * 4 / 1024);

	// iterate through the multiboot memory map and setup the available memory regions
	for(; (uint32_t)entry < mbd->mmap_addr + mbd->mmap_length;)
	{
		if(entry->type == 1)		// this is available memory
			if(phys_mem_free_region((physical_addr)entry->addr, (uint32_t)entry->len - (uint32_t)entry->len % 4096) != ERROR_OK)
				PANIC("Could not insert region into mmb");

		entry = (uint32_t)entry + entry->size + sizeof(entry->size);
	}

	// reserve space for kernel
	uint32_t kernel_length = &kernel_end - &kernel_start;
	if(phys_mem_reserve_region((uint32_t)&__kernel_physical_start, kernel_length + 4096 - kernel_length % 4096) != ERROR_OK)
		PANIC("Could not reserve physical memory for the kernel");
	
	// reserve space for the memory manager
	if(phys_mem_reserve_region(0x200000, phys_mem_get_bitmap_size() + 4096 - phys_mem_get_bitmap_size() % 4096) != ERROR_OK)
		PANIC("Could not reserve physical memory for the memory manager");

	// --------------------------- end: physical memory manager ---------------------------

	// initialize the virtual memory manager

	physical_addr pdir = virt_mem_init(page_dir);
	get_gst()->BSP_dir = pdir;

	virt_mem_map_page(virt_mem_get_current_address_space(), 0x300000, 0x600000, VIRT_MEM_DEFAULT_PDE_FLAGS);
	printfln("tables: %u", virt_mem_count_present_tables(virt_mem_get_current_directory()));

	// This shows how to add a table using recursion mapping
	// care is needed when setting the table to 0

	// physical_addr table_phys = phys_mem_alloc_above_1mb();
	// virtual_addr_t vaddr = 0x900000;

	// uint32_t index = virt_mem_get_page_table_index_by_address(vaddr);
	// printfln("page table index is %u", index);

	// pd_entry* e = &virt_mem_get_current_directory()->entries[index];
	// printfln("page table address: %h", e);

	// pd_entry_add_attrib(e, VIRT_MEM_DEFAULT_PDE_FLAGS);
	// pd_entry_set_frame(e, table_phys);

	// ptable* table = virt_mem_get_page_table(index);
	// memset(table, 0, 4096);

	// PANIC("Setup virtual memory");

	// allocate memory for the acpi resources
	physical_addr x = phys_mem_alloc_above_1mb();		// TODO: Allocate virtual memory
	heap_t* kheap = heap_create(x, 4096);

	// --------------------------- parse acpi tables !!! ---------------------------
	rsdp_descriptor_t* rsdp = rsdp_find();

	if(rsdp == 0)
		PANIC("RSDP could not be found.. TODO: Parse mp tables");

	get_gst()->RSDP_base = rsdp;				// store the rsd pointer for future access

	// perform the enumeration of acpi resources
	if(rsdp_first_parse(rsdp) != 0)
		PANIC("error occured during first rsdp parsing!");

	printfln("acpi resources: lapic: %h, rsdp: %h, processors: %u", get_gst()->lapic_base, get_gst()->RSDP_base, get_gst()->processor_count);

	void* addr;
	// allocate memory for per cpu data
	// TODO: Consider cache line alignment
	if((addr = heap_alloc_a(kheap, get_gst()->processor_count * sizeof(per_cpu_data_t), 4)) == 0)
		PANIC("per cpu data heap allocation failed");
	else
		get_gst()->per_cpu_data_base = (per_cpu_data_t*)addr;

	// allocate memory for the gdt entries (5? permanent for the kernel and the user and one for each processor, see below)
	if((addr = heap_alloc_a(kheap, GDT_GENERAL_ENTRIES + get_gst()->processor_count * sizeof(gdt_entry_t), 4)) == 0)
		PANIC("gdt entries heap allocation failed");
	else
		get_gst()->gdt_entries = (gdt_entry_t*)addr;

	// ensure i/o apic count == 1
	if(get_gst()->ioapic_count != 1)
		PANIC("ioapic count != 1. System not ready for this case");

	printfln("current stack: %h", get_stack());
	PANIC("");

	// using the memory previously allocated, fill in the acpi data structures (mainly per_cpu_data)
	if(rsdp_parse(rsdp) != 0)						// <<<--- TODO: We get exception 13
		PANIC("error occured during rsdp parsing!");

	// --------------------------- end: parse acpi tables !!! ---------------------------

	// --------------------------- setup GDTs ---------------------------
	gdt_set_gate(get_gst()->gdt_entries, 0, 0, 0, 0, 0);
	gdt_set_gate(get_gst()->gdt_entries, 1, 0, 0xFFFFFFFF, GDT_RW | GDT_EX, GDT_SZ | GDT_GRAN);
	gdt_set_gate(get_gst()->gdt_entries, 2, 0, 0xFFFFFFFF, GDT_RW, GDT_SZ | GDT_GRAN);				// code and data entries are common for all processors

	gdt_set_gate(get_gst()->gdt_entries, 3, 0, 0, GDT_RW | GDT_USER, GDT_SZ | GDT_GRAN);		// these are here for the userspace (not yet implemented)
	gdt_set_gate(get_gst()->gdt_entries, 4, 0, 0, GDT_RW | GDT_USER, GDT_SZ | GDT_GRAN);

	// setup a gdt entry for local cpu data (the gs segment will point to these) for each processor
	for(uint32_t i = 0; i < get_gst()->processor_count; i++)
	{
		uint32_t base = (uint32_t)&get_gst()->per_cpu_data_base[i];
		gdt_set_gate(get_gst()->gdt_entries, GDT_GENERAL_ENTRIES + i, base, sizeof(per_cpu_data_t), GDT_RW, GDT_SZ);
	}

	gdt_print_gate(get_gst()->gdt_entries, 5);
	gdt_print_gate(get_gst()->gdt_entries, 6);

	// intstall the new gdt structure
	gdtr_install(get_gst()->gdt_entries, GDT_GENERAL_ENTRIES + get_gst()->processor_count, &get_gst()->gdtr);
	printfln("gdts are set");

	// --------------------------- end: setup GDTs---------------------------

	// --------------------------- boot all processors ---------------------------
	
	// boot each processor (except for the current one which is already running ...)

	_set_cpu_gs(GDT_GENERAL_ENTRIES * 8);
	lapic_enable(get_gst()->lapic_base);
	lapic_calibrate_timer(get_gst()->lapic_base, 10, 64);

	INT_ON;
	
	lock = 1;
	ClearScreen();

	printfln("processor 0 is awake");
	startup_all_AP();


	printfln("******** all processors booted ********");
	lock = 0;
	// --------------------------- end: boot all processors ---------------------------

	while(1)
	{
		acquire_lock(&lock);

		int tempX = cursorX, tempY = cursorY;
		SetPointer(0, SCREEN_HEIGHT - 2);

		printf("time: %u %u", lapic_millis(), per_cpu_read(PER_CPU_OFFSET(lapic_period)));

		SetPointer(tempX, tempY);

		release_lock(&lock);

		for(int i = 0; i < 10000; i++);		// do some random sleep to allow the other processor to acquire the lock
	}
}