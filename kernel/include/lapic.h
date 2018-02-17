#ifndef LAPIC_H_15022018
#define LAPIC_H_15022018

#include <types.h>

enum LAPIC_REGS 
{
    LAPIC_ID = 0x20,
    LAPIC_EOI = 0xB0,
    LAPIC_TIMER = 0x320,
    LAPIC_LINT0 = 0x350,
    LAPIC_INT1 = 0x360,
    LAPIC_ICR_LOW = 0x300,          // Interrupt Command Register
    LAPIC_ICR_HIGH = 0x300,
};

void lapic_enable(physical_addr base_addr);

uint32_t lapic_get_id(physical_addr base_addr);

void lapic_send_eoi(physical_addr base_addr);

#endif