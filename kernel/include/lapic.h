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
    LAPIC_ICR_HIGH = 0x310,
};

enum LAPIC_ICR
{
    LAPIC_ICR_DELIVERY = 7 << 8,
    LAPIC_ICR_DESTINATION = 1 << 11,
    LAPIC_ICR_PENDING = 1 << 12,
    LAPIC_ICR_DESTINATION_TYPE = 3 << 18
};

enum LAPIC_ICR_DELIVERY
{
    LAPIC_DELIVERY_NORMAL = 0,
    LAPIC_DELIVERY_LOWEST = 1,
    LAPIC_DELIVERY_SMI = 2,
    LAPIC_DELIVERY_NMI = 4,
    LAPIC_DELIVERY_INIT = 5,
    LAPIC_DELIVERY_SIPI = 6
};

enum LAPIC_ICR_DESTINATION_TYPE
{
    LAPIC_DESTINATION_TARGET = 0,
    LAPIC_DESTINATION_SELF = 1,
    LAPIC_DESTINATION_ALL = 2,
    LAPIC_DESTINATION_ALL_BUT_SELF = 3
};

void lapic_enable(physical_addr base_addr);

uint32_t lapic_get_id(physical_addr base_addr);

void lapic_send_eoi(physical_addr base_addr);

// send inter-processor interrupt
void lapic_send_ipi(physical_addr base_addr, uint8_t target_id, uint8_t target_vector, uint32_t delivery_mode, uint32_t destination_mode, uint32_t destination_type);

#endif