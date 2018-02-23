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

#include <ap_boot.h>

extern uint32_t pit_count;

gst_t gst;
extern void _print_A();
volatile int ready = 0;

void setup_processor()
{
	ready = 1;
	_print_A();

	while(1);
}

void kernel_main(multiboot_info_t* mbd, unsigned int magic)
{
	SetForegroundColor(VGA_COLOR_GREEN);
	SetBackgroundColor(VGA_COLOR_BLACK);
	ClearScreen();

	Print("Hello this is RadixOS\n");

	gdt_set_gate(0, 0, 0, 0, 0);
	gdt_set_gate(1, 0, 0xFFFFFFFF, GDT_RW | GDT_EX, GDT_SZ | GDT_GRAN);
	gdt_set_gate(2, 0, 0xFFFFFFFF, GDT_RW, GDT_SZ | GDT_GRAN);

	gdt_set_gate(3, 0, 0, GDT_RW, GDT_SZ | GDT_GRAN);		// these are here for the user (not yet implemented)
	gdt_set_gate(4, 0, 0, GDT_RW, GDT_SZ | GDT_GRAN);

	gdtr_install(5);

	idt_init();
	idtr_install();

	// disable the PIC so we can use the local apic
	pic_disable();

	// initialize physical memory manager
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

	memset(&gst, 0, sizeof(gst_t));
	gst.per_cpu_data_base = 0x10000;			// fix this!!

	// parse acpi tables !!!
	rsdp_descriptor_t* rsdp = rsdp_find();

	if(rsdp != 0)
		rsdp_print(rsdp);
	else
		PANIC("RSDP could not be found.. TODO: Parse mp tables");

	if(rsdp_parse(rsdp) != 0)
		PANIC("error occured during rsdp parsing!");

	// test part that enables the 2nd cpu, redirects it to the "setup_processor" function and uses some per cpu data

	pit_timer_init(1000);
	lapic_enable(gst.lapic_base);
	ioapic_map_irq(gst.ioapic_base, 0, 2, 64);

	printfln("gst: %u %h %h %h", gst.processor_count, gst.lapic_base, gst.ioapic_base, gst.per_cpu_data_base);

	per_cpu_data_t* cpu_data = (per_cpu_data_t*)gst.per_cpu_data_base + 1;		// skip the current processor that is already initialized

	while(cpu_data->id != 0)
	{
		// initialize processor
		printfln("cpu id: %u", cpu_data->id);
		gdt_set_gate(4 + cpu_data->id, (uint32_t)cpu_data, sizeof(per_cpu_data_t), GDT_RW, GDT_SZ);

		cpu_data++;
	}

	// reinstall gdt
	gdtr_install(6 + gst.processor_count);

	_set_cpu_gs(5 * 8);
	per_cpu_write(PER_CPU_OFFSET(test_data), 10);
	printfln("value at gs: %u", per_cpu_read(PER_CPU_OFFSET(test_data)));

	ClearScreen();
	asm("sti");
	extern gdt_ptr_t gdtr;

	memcpy(0x7000, (uint32_t*)gdtr.base, 3 * sizeof(gdt_entry_t));

	memcpy(0x8000, ap_boot_bin, ap_boot_bin_len);
	*(uint32_t*)0x8002 = setup_processor;
	*(gdt_ptr_t*)0x8006 = gdtr;
	printfln("length: %u", ap_boot_bin_len);

	ready = 0;
	lapic_send_ipi(gst.lapic_base, 1, 0, LAPIC_DELIVERY_INIT, 0, 0);
	pit_sleep(101);

	printfln("init sent ");

	lapic_send_ipi(gst.lapic_base, 1, 8, LAPIC_DELIVERY_SIPI, 0, 0);

	pit_sleep(2);

	while(!ready){}

	printfln("processor awaken");

	//////////
	while(1)
	{
		pit_sleep(1000);
		printf("| ");
	}

	while(1)
	{
		int tempX = cursorX, tempY = cursorY;
		SetPointer(0, SCREEN_HEIGHT - 2);
		printfln("time: %u", pit_count);

		SetPointer(tempX, tempY);
	}
}