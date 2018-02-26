#ifndef GST_H_21022018
#define GST_H_21022018

#include <types.h>
#include <gdt.h>
#include <idt.h>
#include <acpi.h>
#include <per_cpu_data.h>

/* Defines the Global System Table */

typedef struct global_system_table_struct_t
{
    uint32_t processor_count;
    uint32_t ioapic_count;
    physical_addr lapic_base;
    physical_addr ioapic_base;

    gdt_ptr_t gdtr;                         // gdtr base (since gdt is shared among processors)
    idt_ptr_t idtr;                         // idtr base

    rsdp_descriptor_t* RSDP_base;           // base of the RSDP acpi table

    gdt_entry_t* gdt_entries;               // global descriptor table entries (variable size to account for per processor data)
    per_cpu_data_t* per_cpu_data_base;      // base address of the per cpu data area
    
} volatile gst_t;

// get the global system table
gst_t* get_gst();




#endif