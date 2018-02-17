#ifndef IOAPIC_H_17022018
#define IOAPIC_H_17022018

#include <types.h>

enum IOAPIC_REGS
{
    IOAPIC_INDEX = 0,           // index register used to access other registers
    IOAPIC_DATA = 0x10,         // data register used to communicate data to/from the selected register

    // the following registers need to be selected by the INDEX to become accessible
    IOAPIC_REDTBL0 = 0x10            // irq redirection entries
};

// read data from register 'reg'
uint32_t ioapic_read(void* ioapic_base, uint32_t reg);

// write data to register 'reg'
void ioapic_write(void* ioapic_base, uint32_t reg, uint32_t val);

// maps the given 'irq' to the given 'vector' number that will be routed to the 'apic_id' processor
void ioapic_map_irq(void* ioapic_base, uint32_t apic_id, uint8_t irq, uint8_t vector);

#endif