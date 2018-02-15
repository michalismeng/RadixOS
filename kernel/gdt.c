#include <gdt.h>
#include <debug.h>

gdt_entry_t gdt_entries[5];
gdt_ptr_t gdtr;

extern void _flushGDT(void* gdtr);

void gdt_set_gate(uint16_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags)
{
	gdt_entries[num].base_low = base & 0x0000FFFF;
	gdt_entries[num].base_middle = (base >> 16) & 0x000000FF;
	gdt_entries[num].base_high = (base >> 24) & 0x000000FF;

	gdt_entries[num].limit_low = limit & 0xFFFF;
	gdt_entries[num].flags = (limit >> 16) & 0x0F;	// first 4 bits of flags go to limit_"high"

	gdt_entries[num].flags |= flags & 0xF0;
	gdt_entries[num].access = access | (1 << 7) | (1 << 4);		// OR with Present and always1 bit
}

void gdtr_install()
{
	gdtr.base = (uint32_t)gdt_entries;
	gdtr.limit = 5 * sizeof(gdt_entry_t) - 1;

	_flushGDT(&gdtr);
}

void gdt_print_gate(uint16_t num)
{
	uint32_t base, limit, access, flags;
	base = gdt_entries[num].base_low | (gdt_entries[num].base_middle << 16) | (gdt_entries[num].base_high << 24);
	limit = gdt_entries[num].limit_low | ((uint32_t)(gdt_entries[num].flags & 0x0F) << 16);
	access = gdt_entries[num].access;
	flags = (gdt_entries[num].flags & 0xF0) >> 4;

	printf("gate: %u base: %x, limit: %x, access: %x, flags: %x\n", num, base, limit, access, flags);
}
