#include <ioapic.h>

uint32_t ioapic_read(physical_addr_t ioapic_base, uint32_t reg)
{
    uint8_t volatile* base = (uint8_t volatile*)ioapic_base;
    deref32(base + IOAPIC_INDEX) = reg & 0xFF;                  // write to the 32-bit index register

    return deref32(base + IOAPIC_DATA);
}

void ioapic_write(physical_addr_t ioapic_base, uint32_t reg, uint32_t val)
{
    uint8_t volatile* base = (uint8_t volatile*)ioapic_base;
    deref32(base + IOAPIC_INDEX) = reg & 0xFF;                  // write to the 32-bit index register

    deref32(base + IOAPIC_DATA) = val;
}

void ioapic_map_irq(physical_addr_t ioapic_base, uint32_t apic_id, uint8_t irq, uint8_t vector, uint32_t delivery_mode, uint32_t destination_mode, uint32_t int_mode)
{
    // each redirection register is 64-bits wide, so we split in two 32-bit data
    uint32_t redirect_low = IOAPIC_REDTBL0_LOW + irq * 2;
    uint32_t redirect_high = IOAPIC_REDTBL0_LOW + irq * 2 + 1;

    // first we set the irq destination apic (use the high index bits 56-59)  -- for more bits we need to use clusters (logical mode)
    uint32_t data = ioapic_read(ioapic_base, redirect_high);

    // we only use physical mode
    if(apic_id >= 16)
        PANIC("APIC ID cannot use more that 4 bits");

    data &= 0xFF000000;          // clear any previous value
    data |= (apic_id << 24);     // set the apic target

    ioapic_write(ioapic_base, redirect_high, data);

    // then we adjust various fields of the low index
    data = ioapic_read(ioapic_base, redirect_low);

    if(vector < 0x10 || vector > 0xFE)
        PANIC("IOAPIC was given out of bounds vector");

    data &= ~0xFFFF;        // clear the low 16 bits as we will reset them (the rest is reserved)

    // set the vector number (must be in rage 0x10 to 0xFE)
    data |= vector;

    // set the delivery mode
    data |= delivery_mode;

    // set destination mode
    data |= destination_mode;

    // set interrupt mode (polarity and trigger mode)
    data |= int_mode;

    // unmask the interrupt
    data &= ~IOAPIC_INT_MASK;

    ioapic_write(ioapic_base, redirect_low, data);
}