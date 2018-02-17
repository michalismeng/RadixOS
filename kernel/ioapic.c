#include <ioapic.h>

uint32_t ioapic_read(void* ioapic_base, uint32_t reg)
{
    uint8_t volatile* base = (uint8_t volatile*)ioapic_base;
    deref32(base + IOAPIC_INDEX) = reg & 0xFF;                  // write to the 32-bit index register

    return deref32(base + IOAPIC_DATA);
}

void ioapic_write(void* ioapic_base, uint32_t reg, uint32_t val)
{
    uint8_t volatile* base = (uint8_t volatile*)ioapic_base;
    deref32(base + IOAPIC_INDEX) = reg & 0xFF;                  // write to the 32-bit index register

    deref32(base + IOAPIC_DATA) = val;
}

void ioapic_map_irq(void* ioapic_base, uint32_t apic_id, uint8_t irq, uint8_t vector)
{
    // each redirection register is 64-bits wide, so we split in two 32-bit data
    uint32_t redirect_low = IOAPIC_REDTBL0 + irq * 2;
    uint32_t redirect_high = IOAPIC_REDTBL0 + irq * 2 + 1;

    // first we set the irq destination apic (use the high index bits 56-63)
    uint32_t data = ioapic_read(ioapic_base, redirect_high);

    data &= 0xFF000000;          // clear any previous value
    data |= (apic_id << 24);     // set the apic target

    ioapic_write(ioapic_base, redirect_high, data);

    // then we adjust various fields of the low index
    data = ioapic_read(ioapic_base, redirect_low);

    // set the vector number
    data &= 0xFFFFFF00;             // clear any previous values
    data |= vector;

    // set the interrupt type to 000
    data &= ~0x700;

    // set destination mode to delivery
    data &= ~(1 << 11);

    // unmask the interrupt
    data &= ~(1 << 16);

    ioapic_write(ioapic_base, redirect_low, data);
}