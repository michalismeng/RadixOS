#include <screen.h>
#include <gdt.h>
#include <debug.h>
#include <acpi.h>
#include <pic.h>
#include <multiboot.h>

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

	// disable the PIC so we can use the local apic
	pic_disable();

	printfln("Down once more to the dungeons, %u %h %c", 10, 10, 'h');

	if((mbd->flags & 1) == 0)
		PANIC("Cannot get memory length");
	if((mbd->flags && (1 << 6) == 0))
		PANIC("Cannot get memory map");

	printfln("GRUB detected %u KB lower and %u MB higher", mbd->mem_lower, mbd->mem_upper / 1024);

	multiboot_memory_map_t* entry = (multiboot_memory_map_t*)(mbd->mmap_addr);
	for(; (uint32_t)entry < mbd->mmap_addr + mbd->mmap_length;)
	{
		printfln("entry: %h %u %u", (uint32_t)entry->addr, (uint32_t)entry->len, entry->type);
		entry = (uint32_t)entry + entry->size + sizeof(entry->size);
	}

	// parse acpi tables !!!
	rsdp_descriptor_t* rsdp = rsdp_find();

	if(rsdp != 0)
		rsdp_print(rsdp);
	else
		PANIC("RSDP could not be found.. TODO: Parse mp tables");

	if(rsdp_parse(rsdp) != 0)
		PANIC("error occured during rsdp parsing");
}