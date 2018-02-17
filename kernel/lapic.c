#include <lapic.h>
#include <system.h>

void lapic_enable(physical_addr base_addr)
{
    printfln("enabling lapic att address: %h", base_addr);
    volatile uint32_t val = reg_readl(base_addr, 0xF0);
    val |= (1 << 8);

    // setup the spurious interrupt vector register
    reg_writel(base_addr, 0xF0, val);
}

uint32_t lapic_get_id(physical_addr base_addr)
{
    return reg_readl(base_addr, LAPIC_ID);
}

void lapic_send_eoi(physical_addr base_addr)
{
    // write the value of 0 to the EOI register
    reg_writel(base_addr, LAPIC_EOI, 1);
}
