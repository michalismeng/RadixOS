#include <tss.h>
#include <utility.h>


void tss_init_entry(gdt_entry_t* gdt_base, tss_entry_t* tss, uint16_t num)
{
    uint32_t base = (uint32_t)tss;
    uint32_t limit = base + sizeof(tss_entry_t);

    gdt_base[num].base_low = base & 0x0000FFFF;
	gdt_base[num].base_middle = (base >> 16) & 0x000000FF;
	gdt_base[num].base_high = (base >> 24) & 0x000000FF;
	gdt_base[num].limit_low = limit & 0xFFFF;
	gdt_base[num].flags = (limit >> 16) & 0x0F;
	gdt_base[num].access = 0xE9;

    memset(tss, 0, sizeof(tss_entry_t));

    tss->ss0 = 0x10;

    tss->cs = 0x0B;
    tss->ss = tss->ds = tss->es = tss->fs = 0x13;
    // TODO: Proper care for the GS selector
}

void tss_install(uint16_t num)
{
    _flushTSS(num * 8 + 3);
}

void tss_set_kernel_stack(tss_entry_t* tss, uint32_t stack_top)
{
    tss->esp0 = stack_top;
}