#include <acpi.h>
#include <string.h>
#include <debug.h>
#include <memcmp.h>
#include <gst.h>
#include <per_cpu_data.h>

#include <mem_manager_virt.h>

/* Private Functions */

uint8_t sum(void* base, uint32_t length)
{
    uint8_t sum = 0;
    for(uint32_t i = 0; i < length; i++)
        sum += *((uint8_t*)base + i);

    return sum;
}

// search for 'length' bytes beginning from base to find a valid RSDP struct
rsdp_descriptor_t* rsdp_find_in(physical_addr base, uint32_t length)
{
    // search at a 16-byte boundary (assume base is well-aligned)
    for(uint32_t i = 0; i < length; i += 16)
    {
        rsdp_descriptor_t* rsdp = (rsdp_descriptor_t*)(base + i);

        if(memcmp(rsdp->signature, "RSD PTR ", 8) == 0 && sum(rsdp, sizeof(rsdp_descriptor_t)) == 0)
            return rsdp;
    }

    return 0;
}

// perform first pass (enumeration) of a MADT entry
int madt_first_parse(acpi_dt_header_t* header)
{
    madt_descriptor_t* madt = (madt_descriptor_t*)header;
    
    get_gst()->lapic_base = madt->lapic_addr;

    // parse each entry of the madt table
    uint8_t* start = (uint8_t*)(madt + 1);                      // the entries start at the end of the madt
    uint8_t* end = (uint8_t*)madt + madt->acpi_header.length;   // the entries end is defined by the length of the acpi header

    for(uint8_t* ptr = start; ptr < end;)
    {
        madt_entry_header_t* madt_entry = (madt_entry_header_t*)ptr;

        // enumerate system resources
        switch(madt_entry->entry_type)
        {
            case LAPIC:
                get_gst()->processor_count++;
                break;
            case IOAPIC:
                get_gst()->ioapic_count++;
                break;
            default:
                break;                      
        }

        ptr += madt_entry->entry_length;    
    }

    return 0;
}

// parses a madt (APIC) entry
int madt_parse(acpi_dt_header_t* header)
{
    madt_descriptor_t* madt = (madt_descriptor_t*)header;
    
    //parse each entry of the madt table
    uint8_t* start = (uint8_t*)(madt + 1);                      // the entries start at the end of the madt
    uint8_t* end = (uint8_t*)madt + madt->acpi_header.length;   // the entries end is defined by the length of the acpi header
    uint32_t cpu_index = 0;

    for(uint8_t* ptr = start; ptr < end;)
    {
        madt_entry_header_t* madt_entry = (madt_entry_header_t*)ptr;

        if(madt_entry->entry_type == LAPIC)
        {
            madt_lapic_descriptor_t* lapic = (madt_lapic_descriptor_t*)madt_entry;
            if(lapic->processor_id != lapic->apic_id)
                PANIC("found processor id != local apic id");

            per_cpu_data_t cpu_data;

            cpu_data.id = lapic->processor_id;
            cpu_data.enabled = lapic->flags;
            cpu_data.test_data = 150 + lapic->processor_id;

            // set the cpu data to its local storage
            gst_add_processor(cpu_index++, lapic->processor_id, cpu_data);
        }
        else if(madt_entry->entry_type == IOAPIC)
        {
            madt_ioapic_descriptor_t* ioapic = (madt_ioapic_descriptor_t*)madt_entry;
            get_gst()->ioapic_base = ioapic->ioapic_addr;

        }
        else if(madt_entry->entry_type == INTERRUPT_OVERRIDE)
        {
            // assume all interrupts are Edge Trigerred and Active High
            // TODO: Maintain a list of information foreach of the IO APICs pins.
            // TODO: Use AML to parse PCI interrupt information
            madt_interrupt_override_descriptor_t* override = (madt_interrupt_override_descriptor_t*)madt_entry;
            gst_set_int_override(override->irq_source, override->global_system_int);
        }
        else
            printfln("madt entry: %u", madt_entry->entry_type);

        ptr += madt_entry->entry_length;
    }

    return 0;
}

/* Public Functions */

rsdp_descriptor_t* rsdp_find()
{
    rsdp_descriptor_t* rsdp = 0;

    // search in the end of conventional memory

    if((rsdp = rsdp_find_in(0xE0000, 0x20000)))        
        return rsdp;
    else
        printfln("rsdt not found in the end of conventional memory");

    uint16_t* ebda_ptr = 0x400 + 0x0E;      // pointer to the EBDA
    uint16_t* base_mem_ptr = 0x400 + 0x13;  // pointer to the end of base memory
	uint32_t data;

    // search in the first KB of EBDA

	if((data = *ebda_ptr) != 0)
	{
		data <<= 4;

		if((rsdp = rsdp_find_in(data, 1024)))       
            return rsdp;
		else		                                // search in the end of base memory (again 1KB search)
            printfln("rsdt not found in the EBDA");
	}
	else
		printf(" ebda not present.");

    return 0;
}

int rsdp_first_parse(rsdp_descriptor_t* rsdp)
{
    // require valid rsdp and acpi version 1 (which is implied by "version == 0" !!)
    if(rsdp == 0 || rsdp->revision != 0)
        return 1;

    rsdt_descriptor_t* rsdt = (rsdt_descriptor_t*)rsdp->rsdt_addr;

    // map the page for usage
    if(!virt_mem_is_page_present((uint32_t)rsdt & (~0xfff)))
        virt_mem_map_page(virt_mem_get_current_address_space(), (uint32_t)rsdt & (~0xfff), (uint32_t)rsdt & (~0xfff), VIRT_MEM_DEFAULT_PTE_FLAGS);

	uint32_t entries = (rsdt->acpi_header.length - sizeof(acpi_dt_header_t)) / 4;

    for(uint32_t i = 0; i < entries; i++)
	{
		acpi_dt_header_t* header = (acpi_dt_header_t*)rsdt->sdt_ptrs[i];

		if(memcmp(header->signature, ACPI_MADT, 4) == 0)
        {
            if(madt_first_parse(header) != 0)
                return 1;
        }
        else if(memcmp(header->signature, ACPI_HPET, 4) == 0)
        {
            printfln("found hpet");
            hpet_descriptor_t* hpet = (hpet_descriptor_t*)header;

            printfln("hpet: %h %u", (uint32_t)hpet->address, hpet->comparator_count);
        }
        else
            printfln("%s", header->signature);
        // do first parse for other tables: FADT, SLIT...
	}

    return 0;
}

int rsdp_parse(rsdp_descriptor_t* rsdp)
{
    // require valid rsdp and acpi version 1 (which is implied by "version == 0" !!)    
    if(rsdp == 0 || rsdp->revision != 0)
        return 1;

    rsdt_descriptor_t* rsdt = (rsdt_descriptor_t*)rsdp->rsdt_addr;
	uint32_t entries = (rsdt->acpi_header.length - sizeof(acpi_dt_header_t)) / 4;

	for(uint32_t i = 0; i < entries; i++)
	{
		acpi_dt_header_t* header = (acpi_dt_header_t*)rsdt->sdt_ptrs[i];

		if(memcmp(header->signature, ACPI_MADT, 4) == 0)
        {
            if(madt_parse(header) != 0)
                return 1;
        }
        // do other checks for FADT, SLIT...
	}

    return 0;
}

void rsdp_print(rsdp_descriptor_t* rsdp)
{
    printfln("%c%c%c%c%c%c%c%c rev: %u addr: %h",
                        rsdp->signature[0],
                        rsdp->signature[1],
                        rsdp->signature[2],
                        rsdp->signature[3],
                        rsdp->signature[4],
                        rsdp->signature[5],
                        rsdp->signature[6],
                        rsdp->signature[7], 
                        rsdp->revision, rsdp->rsdt_addr);
}