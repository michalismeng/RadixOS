#ifndef PER_CPU_DATA_H_23022018
#define PER_CPU_DATA_H_23022018

#include <types.h>
#include <tss.h>
#include <thread_sched.h>

extern void _set_cpu_gs(uint32_t gdt_offset);
extern void _set_cpu_ss(uint32_t gdt_offset);

// return the offset of a field in the cpu data sturcture
#define PER_CPU_OFFSET(field) offsetof(per_cpu_data_t, field)

// macros for quick access to cpu related data
#define get_cpu_id ( per_cpu_read(PER_CPU_OFFSET(id)) )
#define get_cpu_stack ( per_cpu_read(PER_CPU_OFFSET(common_stack_top)) )
#define cpu_is_bsp ( per_cpu_read(PER_CPU_OFFSET(id)) == 0 )            // cpu with id 0 is the BSP


// defines the data, local to each processor
typedef volatile struct per_cpu_data_struct_t
{
    // TODO: Investiagte ID ranges (are there holes?)
    uint32_t id;                    // the processor id
    uint32_t enabled;               // if set => processor can startup, otherwise do not initiate boot sequence
    uint32_t test_data;             // random test data to check if gs segment addressing works
    uint32_t lapic_period;          // period of the lapic timer
    uint32_t lapic_count;           // count of the lapic timer
    uint32_t common_stack_top;      // common stack used by user threads when context switching

    tss_entry_t tss_entry;          // tss data for the processor

    thread_sched_t scheduler;       // cpu scheduler

    // TODO: add other cpu local stuff...

} per_cpu_data_t;

// read a local cpu data at the given offset
uint32_t per_cpu_read(uint32_t offset);

// write to a local cpu data at the given offset
void per_cpu_write(uint32_t offset, uint32_t value);

#endif