#include <screen.h>
#include <gdt.h>
#include <debug.h>
#include <acpi.h>
#include <pic.h>
#include <multiboot.h>
#include <phys_mem_manager.h>
#include <idt.h>
#include <ioapic.h>

extern void* lapic_base;
extern void* ioapic_base;
extern volatile int count;

// TODO: Move these to a proper file
#define TERMINAL_COUNT			0 << 1
#define PROG_ONESHOT			1 << 1
#define RATE_GENERATOR			2 << 1	// used for the timer
#define SQR_WAVE_GENERATOR		3 << 1	// used for the speaker
#define SOFT_TRIGGER			4 << 1
#define HARD_TRIGGER			5 << 1

#define LATCH					0 << 4
#define LSB_ONLY				1 << 4		// least significand byte
#define MSB_ONLY				2 << 4		// most significand byte only
#define LSB_THEN_MSB			3 << 4

#define COUNTER_0				0 << 6
#define COUNTER_1				1 << 6
#define COUNTER_2				2 << 6

void init_pit_timer(uint32_t _frequency)
{
	//frequency = _frequency;

	// frequency must be such that count fits in a 16-bit variable.
	uint16_t count = 1193180 / _frequency;
	//initialization command byte

	uint8_t cw = RATE_GENERATOR | LSB_THEN_MSB | COUNTER_0;

	outportb(0x43, cw);

	uint8_t l = (uint8_t)(count & 0x00FF);
	uint8_t h = (uint8_t)((count >> 8) & 0x00FF);

	outportb(0x40, l);	// send the least significand byte first
	outportb(0x40, h);	// then send the most.
}

/////////////////////////////////////////////////////

void kernel_main(multiboot_info_t* mbd, unsigned int magic)
{
	SetForegroundColor(VGA_COLOR_GREEN);
	SetBackgroundColor(VGA_COLOR_BLACK);
	ClearScreen();

	Print("Hello this is RadixOS\n");

	gdt_set_gate(0, 0, 0, 0, 0);
	gdt_set_gate(1, 0, 0xFFFFFFFF, GDT_RW | GDT_EX, GDT_SZ | GDT_GRAN);
	gdt_set_gate(2, 0, 0xFFFFFFFF, GDT_RW, GDT_SZ | GDT_GRAN);

	gdtr_install();

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

	// parse acpi tables !!!
	rsdp_descriptor_t* rsdp = rsdp_find();

	if(rsdp != 0)
		rsdp_print(rsdp);
	else
		PANIC("RSDP could not be found.. TODO: Parse mp tables");

	if(rsdp_parse(rsdp) != 0)
		PANIC("error occured during rsdp parsing!");

	init_pit_timer(1000);
	lapic_enable(lapic_base);
	ioapic_map_irq(ioapic_base, 0, 2, 64);

	ClearScreen();
	asm("sti");

	while(1)
	{
		int tempX = cursorX, tempY = cursorY;
		SetPointer(0, SCREEN_HEIGHT - 1);
		printfln("time: %u", count);

		SetPointer(tempX, tempY);
	}
}