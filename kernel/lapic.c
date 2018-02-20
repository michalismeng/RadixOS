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

void lapic_send_ipi(physical_addr base_addr, uint8_t target_id, uint8_t target_vector, uint32_t delivery_mode, uint32_t destination_mode, uint32_t destination_type)
{
    // wait until there are no pending IPIs
    while(reg_readl(base_addr, LAPIC_ICR_LOW) & LAPIC_ICR_PENDING);

    // set the high ICR at first, as writing to the low one triggers the IPI
    reg_writel(base_addr, LAPIC_ICR_HIGH, (target_id & 0x0F) << 24);

    uint32_t low = target_vector;
    low |= (delivery_mode << 8) & LAPIC_ICR_DELIVERY;
    low |= (destination_mode << 11) & LAPIC_ICR_DESTINATION;

    if(delivery_mode == LAPIC_DELIVERY_INIT)
    {
        low &= ~(1 << 14);
        low &= ~(1 << 15);
    }
    else
        low |= (1 << 14);

	low |= (destination_type << 18) & LAPIC_ICR_DESTINATION_TYPE;

    reg_writel(base_addr, LAPIC_ICR_LOW, low);
}