#ifndef GST_H_21022018
#define GST_H_21022018

#include <types.h>
#include <gdt.h>
#include <idt.h>
#include <acpi.h>
#include <per_cpu_data.h>

#define MAX_CPUS    256         // maximum cpus the kernel can handle (acpi 1.0 defines processor id as 8 bits)

/* Defines the Global System Table */

typedef struct global_system_table_struct_t
{
    uint32_t processor_count;
    uint32_t ioapic_count;
    physical_addr lapic_base;
    physical_addr ioapic_base;

    gdt_ptr_t gdtr;                             // gdtr base (since gdt is shared among processors)
    idt_ptr_t idtr;                             // idtr base

    rsdp_descriptor_t* RSDP_base;               // base of the RSDP acpi table

    uint8_t ioapic_overrides[16];               // overrides of legacy interrupts to the io apic (16 legacy interrupts in total)

    gdt_entry_t* gdt_entries;                   // global descriptor table entries (variable size to account for per processor data)
    per_cpu_data_t* per_cpu_ptrs[MAX_CPUS];     // pointers 
    per_cpu_data_t* per_cpu_data_base;          // base address of the per cpu data area
    
} volatile gst_t;

// get the global system table
gst_t* get_gst();

// add processor data to the global system table
int gst_add_processor(uint32_t cpu_index, uint32_t processor_id, per_cpu_data_t processor_data);

// returns a cpu local storage given the processor id
per_cpu_data_t* get_cpu_storage(uint32_t processor_id);

// set interrupt override of legacy PIC to the io apic
void gst_set_int_override(uint8_t irq_source, uint8_t global_system_int);

// get the override mapping that corresponds to the irq_source
uint8_t gst_get_int_override(uint8_t irq_source);


#endif