#ifndef PER_CPU_DATA_H_23022018
#define PER_CPU_DATA_H_23022018

#include <types.h>

// assembly to read and write based on the gs segment register
extern uint32_t _per_cpu_read(uint32_t var_offset);
extern void _per_cpu_write(uint32_t var_offset, uint32_t value);

extern void _set_cpu_gs(uint32_t gdt_offset);

// return the offset of a field in the cpu data sturcture
#define PER_CPU_OFFSET(field) offsetof(per_cpu_data_t, field)

// defines the data, local to each processor
// TODO: Consider cache-line alignment
typedef struct per_cpu_data_struct_t
{
    uint32_t id;
    uint32_t test_data;

    // TODO: add scheduler queues and other cpu local stuff...

} per_cpu_data_t;

// read a local cpu data at the given offset
uint32_t per_cpu_read(uint32_t offset);

// write to a local cpu data at the given offset
void per_cpu_write(uint32_t offset, uint32_t value);

#endif