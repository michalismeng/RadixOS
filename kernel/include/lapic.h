#ifndef LAPIC_H_15022018
#define LAPIC_H_15022018

#include <types.h>

enum LAPIC_REGS 
{
    LAPIC_EOI = 0xB0 
};

void lapic_enable(physical_addr base_addr);

inline uint32_t lapic_get_id(physical_addr base_addr);

inline void lapic_send_eoi(physical_addr base_addr);

#endif