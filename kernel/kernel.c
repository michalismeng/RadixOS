#include <screen.h>
#include <gdt.h>
#include <debug.h>
#include <acpi.h>
#include <pic.h>
#include <multiboot.h>
#include <phys_mem_manager.h>
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

uint32_t lock = 0;

int test_and_set(int new_val, int* lock);

void kernel_main(multiboot_info_t* mbd, unsigned int magic)
{
	SetForegroundColor(VGA_COLOR_GREEN);
	SetBackgroundColor(VGA_COLOR_BLACK);
	ClearScreen();

	Print("Hello this is RadixOS \n");

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
	if(phys_mem_manager_init(0x100000) != 0)
		PANIC("Could not initialize physical memory manager");

	if((mbd->flags & 1) == 0)
		PANIC("Cannot get memory length");
	if((mbd->flags && (1 << 6) == 0))
		PANIC("Cannot get memory map");

	printfln("GRUB detected %u KB lower and %u MB higher", mbd->mem_lower, mbd->mem_upper / 1024);

	multiboot_memory_map_t* entry = (multiboot_memory_map_t*)(mbd->mmap_addr);
	int error = 0;

	for(; (uint32_t)entry < mbd->mmap_addr + mbd->mmap_length;)
	{
		if(entry->type == 1)		// this is available memory
		{
			// MMB is placed at 0x100000, so fix memory to not interfere
			if(entry->addr == 0x100000)
			{
				entry->addr += 0x1000;
				entry->len -= 0x1000;
			}
			if( (error = phys_mem_insert_region((physical_addr)entry->addr, (uint32_t)entry->len - (uint32_t)entry->len % 4096)) != 0)
			{
				printfln("error: %u", error);
				PANIC("Could not insert region into mmb");
			}
		}
			
		entry = (uint32_t)entry + entry->size + sizeof(entry->size);
	}

	phys_mem_print();

	// --------------------------- end: physical memory manager ---------------------------

	// allocated memory for the acpi resources
	heap_t* kheap = heap_create(0x150000, 4096);

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

	// prepare the PIT - not very useful now
	//pit_timer_init(1000, 0);
	//ioapic_map_irq(get_gst()->ioapic_base, 0, gst_get_int_override(0), 224);

	lapic_enable(get_gst()->lapic_base);
	// calibrate the lapic timer of the BSP
	_set_cpu_gs(GDT_GENERAL_ENTRIES * 8);
	lapic_calibrate_timer(get_gst()->lapic_base, 10, 64);

	INT_ON;
	
	lock = 1;
	ClearScreen();

	printfln("processor 0 is awake");
	startup_all_AP();

	printfln("******** all processors booted ********");

	printfln("test lock address: %h", &lock);
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