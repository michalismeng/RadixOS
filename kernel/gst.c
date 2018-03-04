#include <gst.h>

gst_t gst;

inline gst_t* get_gst()
{
    return &gst;
}

int gst_add_processor(uint32_t cpu_index, uint32_t processor_id, per_cpu_data_t processor_data)
{
    if(cpu_index >= get_gst()->processor_count || processor_id >= MAX_CPUS)
        return 1;

    get_gst()->per_cpu_ptrs[processor_id] = &get_gst()->per_cpu_data_base[cpu_index];
    get_gst()->per_cpu_data_base[cpu_index] = processor_data;

    return 0;
}

per_cpu_data_t* get_cpu_storage(uint32_t processor_id)
{
    return &get_gst()->per_cpu_data_base[processor_id];
}

void gst_set_int_override(uint8_t irq_source, uint8_t global_system_int)
{
    get_gst()->ioapic_overrides[irq_source] = global_system_int;
}

uint8_t gst_get_int_override(uint8_t irq_source)
{
    return get_gst()->ioapic_overrides[irq_source];
}