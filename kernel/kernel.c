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
#include <vm_contract.h>

#include <process.h>
uint32_t lock = 0;

heap_t* kheap;

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

	// allocate memory for the acpi resources
	physical_addr x = phys_mem_alloc_above_1mb();		// TODO: Allocate virtual memory
	kheap = heap_create(x, 4096);

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
	if((addr = heap_alloc_a(kheap, (GDT_GENERAL_ENTRIES + get_gst()->processor_count) * sizeof(gdt_entry_t), 4)) == 0)
		PANIC("gdt entries heap allocation failed");
	else
		get_gst()->gdt_entries = (gdt_entry_t*)addr;

	// ensure i/o apic count == 1
	if(get_gst()->ioapic_count != 1)
		PANIC("ioapic count != 1. System not ready for this case");

	// relocate the stack to a safe-ish location
    asm ("movl %0, %%esp"::"r"(phys_mem_alloc_above_1mb() + 4094):"%esp");
	printfln("current stack: %h", get_stack());

	// using the memory previously allocated, fill in the acpi data structures (mainly per_cpu_data)
	if(rsdp_parse(rsdp) != 0)
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

	// ----------------------- test vm contract -------------------------------------
	// ClearScreen();

	// vm_contract_t mem_contract;
	// vm_contract_init(&mem_contract);

	// printfln("success: %u", vm_contract_add_area(&mem_contract, vm_area_create(0, 4095, 1, 0, 0)));
	// printfln("success: %u", vm_contract_add_area(&mem_contract, vm_area_create(0, 1 MB - 1, 0, 0, 0)));
	// printfln("success: %u", vm_contract_add_area(&mem_contract, vm_area_create(4096, 1 MB - 1, 0, 0, 0)));
	// printfln("success: %u", vm_contract_add_area(&mem_contract, vm_area_create(1 MB, 2 MB - 1, 0, 0, 0)));

	// vm_area_t* target = vm_contract_find_area(&mem_contract, 500);
	// printfln("target area: %h - %h", target->start_addr, target->end_addr);

	// vm_contract_print(&mem_contract);

	// PANIC("TEST END...");

	// ----------------------- test vm contract -------------------------------------


	INT_ON;
	
	lock = 1;
	ClearScreen();

	printfln("processor 0 is awake at stack %h", get_stack());
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