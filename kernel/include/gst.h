#ifndef GST_H_21022018
#define GST_H_21022018

#include <types.h>
#include <gdt.h>
#include <idt.h>
#include <acpi.h>

/* Defines the Global System Table */

typedef struct global_system_table_struct_t
{
    uint32_t processor_count;
    physical_addr lapic_base;
    physical_addr ioapic_base;
    gdt_ptr_t gdtr;                      // gdtr base (since gdt is shared among processors)
    idt_ptr_t idtr;

    madt_descriptor_t* MADT_base;        // base of the MADT acpi table

    physical_addr per_cpu_data_base;     // base address of the per cpu data area
    
} gst_t;

extern gst_t gst;

#endif