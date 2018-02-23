#include <per_cpu_data.h>
#include <stddef.h>

uint32_t per_cpu_read(uint32_t offset)
{
    return _per_cpu_read(offset);
}

void per_cpu_write(uint32_t offset, uint32_t value)
{
    _per_cpu_write(offset, value);
}