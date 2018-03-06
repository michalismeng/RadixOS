#ifndef LAPIC_H_15022018
#define LAPIC_H_15022018

#include <types.h>

// APIC_APICID	= 20h
// APIC_APICVER	= 30h
// APIC_TASKPRIOR	= 80h
// APIC_EOI	= 0B0h
// APIC_LDR	= 0D0h
// APIC_DFR	= 0E0h
// APIC_SPURIOUS	= 0F0h
// APIC_ESR	= 280h
// APIC_ICRL	= 300h
// APIC_ICRH	= 310h
// APIC_LVT_TMR	= 320h
// APIC_LVT_PERF	= 340h
// APIC_LVT_LINT0	= 350h
// APIC_LVT_LINT1	= 360h
// APIC_LVT_ERR	= 370h
// APIC_TMRINITCNT	= 380h
// APIC_TMRCURRCNT	= 390h
// APIC_TMRDIV	= 3E0h
// APIC_LAST	= 38Fh
// APIC_DISABLE	= 10000h
// APIC_SW_ENABLE	= 100h
// APIC_CPUFOCUS	= 200h
// APIC_NMI	= (4<<8)
// TMR_PERIODIC	= 20000h
// TMR_BASEDIV	= (1<<20)

enum LAPIC_REGS 
{
    LAPIC_ID = 0x20,
    LAPIC_EOI = 0xB0,
    LAPIC_TIMER_LVT = 0x320,   
    LAPIC_TIMER_DISABLE = 0x10000,      // Write this value to the timer_lvt register to mask the timer
    LAPIC_TIMER_PERIODIC = 0x20000,     // OR this value with the vector number in the timer_lvt register to set periodic interrupt mode      
    LAPIC_LINT0 = 0x350,
    LAPIC_INT1 = 0x360,
    LAPIC_ICR_LOW = 0x300,          // Interrupt Command Register
    LAPIC_ICR_HIGH = 0x310,
    LAPIC_TIMER_DIV = 0x3E0,        // Time Divide Configuration Register
    LAPIC_TIMER_INIT = 0x380,       // Initial Timer Count
    LAPIC_TIMER_CUR = 0x390         // Current Timer Count
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

void lapic_start_timer(physical_addr base_addr);

#endif