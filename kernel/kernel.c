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

#include <ap_boot.h>

volatile int ready = 0;

void setup_processor()
{
	// test AP local data
	printfln("test data unmodified: %u", per_cpu_read(PER_CPU_OFFSET(test_data)));	
	per_cpu_write(PER_CPU_OFFSET(test_data), 400);
	printfln("test data modified: %u", per_cpu_read(PER_CPU_OFFSET(test_data)));

	// give the mark to the main cpu to continue waking up processors
	ready = 1;

	while(1);
}

void final_processor_setup()
{
	// here we do the final setup of all the processors
	// concerning virtual memory - lapic timers etc
}

// startup the processor with 'id' and set it to execute at 'exec_base'
void processor_startup(uint32_t lapic_id, physical_addr exec_base)
{
	// implement intel protocol for processor boot 
	
	// send the INIT interrupt and wait for 100ms
	lapic_send_ipi(get_gst()->lapic_base, lapic_id, 0, LAPIC_DELIVERY_INIT, 0, 0);
	lapic_sleep(100);

	// send the STARTUP interrupt and wait for 1ms
	lapic_send_ipi(get_gst()->lapic_base, lapic_id, exec_base >> 12, LAPIC_DELIVERY_SIPI, 0, 0);
	lapic_sleep(1);
}

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

	// allocate memory for the gdt entries (5 permanent for the kernel and the user and one for each processor, see below)
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
	gdt_set_gate(get_gst()->gdt_entries, 2, 0, 0xFFFFFFFF, GDT_RW, GDT_SZ | GDT_GRAN);

	gdt_set_gate(get_gst()->gdt_entries, 3, 0, 0, GDT_RW | GDT_USER, GDT_SZ | GDT_GRAN);		// these are here for the userspace (not yet implemented)
	gdt_set_gate(get_gst()->gdt_entries, 4, 0, 0, GDT_RW | GDT_USER, GDT_SZ | GDT_GRAN);

	// setup a gdt entry for per cpu data (the gs segment will point to these) for each processor
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
	lapic_enable(get_gst()->lapic_base);
	//ioapic_map_irq(get_gst()->ioapic_base, 0, gst_get_int_override(0), 224);

	// calibrate the lapic timer of the BSP
	_set_cpu_gs(GDT_GENERAL_ENTRIES * 8);
	lapic_calibrate_timer(get_gst()->lapic_base, 10, 64);
	per_cpu_write(PER_CPU_OFFSET(lapic_count), 0);


	asm("sti");
	ClearScreen();
	// this is a hardcoded memory location required by the assembled 'ap_boot.fasm'
	memcpy(0x8000, ap_boot_bin, ap_boot_bin_len);
	*(uint32_t*)0x8002 = setup_processor;
	*(gdt_ptr_t*)0x8006 = get_gst()->gdtr;

	printfln("cpu %u booted", get_gst()->per_cpu_data_base[0].id);
	
	for(uint32_t i = 1; i < get_gst()->processor_count; i++)
	{
		if(get_gst()->per_cpu_data_base[i].enabled == 0)
			continue;

		ready = 0;
		*(uint16_t*)0x800C = i + GDT_GENERAL_ENTRIES;		// set the ID to fix the gs segment
		processor_startup(get_gst()->per_cpu_data_base[i].id, 0x8000);

		// wait for the processor to gracefully boot 
		while(ready == 0);
		printfln("cpu: %u booted ", get_gst()->per_cpu_data_base[i].id);
	}

	printfln("all processors booted");

	// --------------------------- end: boot all processors ---------------------------
	
	while(1)
	{
		int tempX = cursorX, tempY = cursorY;
		SetPointer(0, SCREEN_HEIGHT - 2);

		printfln("time: %u %u", lapic_millis(), pit_millis());

		SetPointer(tempX, tempY);
	}
}