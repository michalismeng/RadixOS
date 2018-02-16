#include <acpi.h>
#include <string.h>
#include <debug.h>
#include <memcmp.h>

#include <lapic.h>

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
    char* sig = "RSD PTR ";
    // search at a 16-byte boundary (assume base is well-aligned)
    for(uint32_t i = 0; i < length; i += 16)
    {
        rsdp_descriptor_t* rsdp = (rsdp_descriptor_t*)(base + i);

        if(memcmp(rsdp->signature, "RSD PTR ", 8) == 0 && sum(rsdp, sizeof(rsdp_descriptor_t)) == 0)
            return rsdp;
    }

    return 0;
}

// parses a madt (APIC) entry
int madt_parse(acpi_dt_header_t* header)
{
    madt_descriptor_t* madt = (madt_descriptor_t*)header;
    printfln("madt: %h %c%c %u", madt, madt->acpi_header.signature[0], madt->acpi_header.signature[1], madt->flags);
    
    //parse each entry of the madt table
    uint8_t* start = (uint8_t*)(madt + 1);                      // the entries start at the end of the madt
    uint8_t* end = (uint8_t*)madt + madt->acpi_header.length;   // the entries end is defined by the length of the acpi header

    for(uint8_t* ptr = start; ptr < end;)
    {
        madt_entry_header_t* madt_entry = (madt_entry_header_t*)ptr;

        if(madt_entry->entry_type == LAPIC)
        {
            madt_lapic_descriptor_t* lapic = (madt_lapic_descriptor_t*)madt_entry;
            printfln("madt LAPIC: %u with processor: %u flags: %u", lapic->apic_id, lapic->processor_id, lapic->flags);
        }
        else if(madt_entry->entry_type == IOAPIC)
        {
            madt_ioapic_descriptor_t* ioapic = (madt_ioapic_descriptor_t*)madt_entry;
            printfln("madt IOAPIC: %u address %h", ioapic->ioapic_id, ioapic->ioapic_addr);
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

    if((rsdp = rsdp_find_in(0xE0000, 0x20000)))        // search in the end of conventional memory
        return rsdp;
    else
        printfln("rsdt not found in the end of conventional memory");

    uint16_t* ebda_ptr = 0x400 + 0x0E;      // pointer to the EBDA
    uint16_t* base_mem_ptr = 0x400 + 0x13;  // pointer to the end of base memory
	uint32_t data;

	if((data = *ebda_ptr) != 0)
	{
		data <<= 4;

		if((rsdp = rsdp_find_in(data, 1024)))       // search in the first KB of EBDA
            return rsdp;
		else		                                // search in the end of base memory (again 1KB search)
            printfln("rsdt not found in the EBDA");
	}
	else
		printf(" ebda not present.");

    return 0;
}

int rsdp_parse(rsdp_descriptor_t* rsdp)
{
    if(rsdp == 0 || rsdp->revision != 0)
        return 1;

    rsdt_descriptor_t* rsdt = (rsdt_descriptor_t*)rsdp->rsdt_addr;
	int entries = (rsdt->acpi_header.length - sizeof(acpi_dt_header_t)) / 4;

	printfln("rsdt entries: %u", entries);

	for(int i = 0; i < entries; i++)
	{
		acpi_dt_header_t* header = (acpi_dt_header_t*)rsdt->sdt_ptrs[i];

		if(memcmp(header->signature, "APIC", 4) == 0)
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