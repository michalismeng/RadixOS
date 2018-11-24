#include <gdt.h>
#include <debug.h>

gdt_entry_t gdt_entries[3];

extern void _flushGDT(void* gdtr);

void gdt_set_gate(gdt_entry_t* gdt_base, uint16_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags)
{
	gdt_base[num].base_low = base & 0x0000FFFF;
	gdt_base[num].base_middle = (base >> 16) & 0x000000FF;
	gdt_base[num].base_high = (base >> 24) & 0x000000FF;

	gdt_base[num].limit_low = limit & 0xFFFF;
	gdt_base[num].flags = (limit >> 16) & 0x0F;					// first 4 bits of flags go to limit_"high"

	gdt_base[num].flags |= flags & 0xF0;
	gdt_base[num].access = access | (1 << 7) | (1 << 4);		// OR with Present and always1 bit
}

void gdtr_install(physical_addr_t gdt_base, uint32_t count, gdt_ptr_t* gdtr)
{
	gdtr->base = (uint32_t)gdt_base;
	gdtr->limit = count * sizeof(gdt_entry_t) - 1;

	_flushGDT(gdtr);
}

void gdt_print_gate(gdt_entry_t* gdt_base, uint16_t num)
{
	uint32_t base, limit, access, flags;
	base = gdt_base[num].base_low | (gdt_base[num].base_middle << 16) | (gdt_base[num].base_high << 24);
	limit = gdt_base[num].limit_low | ((uint32_t)(gdt_base[num].flags & 0x0F) << 16);
	access = gdt_base[num].access;
	flags = (gdt_base[num].flags & 0xF0) >> 4;

	printf("gate: %u base: %x, limit: %x, access: %x, flags: %x\n", num, base, limit, access, flags);
}
