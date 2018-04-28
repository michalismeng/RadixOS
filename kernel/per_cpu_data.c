#include <per_cpu_data.h>
#include <stddef.h>

// assembly to read and write based on the gs segment register
extern uint32_t _per_cpu_read(uint32_t var_offset);
extern void _per_cpu_write(uint32_t var_offset, uint32_t value);

uint32_t per_cpu_read(uint32_t offset)
{
    return _per_cpu_read(offset);
}

void per_cpu_write(uint32_t offset, uint32_t value)
{
    _per_cpu_write(offset, value);
}