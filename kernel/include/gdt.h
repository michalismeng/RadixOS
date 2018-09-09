#ifndef GDT_H_10022018
#define GDT_H_10022018

#include <types.h>

#define GDT_BASE_DUMMY 			gdt_entries			// dummy gdt base for initial kernel setup (may be moved to boot.s)
#define GDT_GENERAL_ENTRIES 	5					// number of entries for the general gdt (null entry, 2 kernel, 2 user, 1 TSS)

enum GDT_FLAGS
{
	GDT_SZ =		1 << 6,			// if set segment is 32-bit protected mode, otherwise 16-bit
	GDT_GRAN =		1 << 7,			// if set segment has 4KB granularity, otherwise byte granularity
};

enum GDT_ACCESS
{
	GDT_RW =		1 << 1,			// read access for executable, write access for data
	GDT_DC =		1 << 2,			// direction bit
	GDT_EX =		1 << 3,			// if set the segment is executable, otherwise is data
	GDT_USER =		3 << 5			// if set then the segment is for userspace, otherwise it is supervisor
};

#pragma pack(push, 1)

typedef struct gdt_entry_struct_t
{
	uint16_t limit_low;		// The lower 16 bits of the limit.
	uint16_t base_low;		// The lower 16 bits of the base.
	uint8_t base_middle;	// The next 8 bits of the base.
	uint8_t access;			// Access flags, determine what ring this segment can be used in.
	uint8_t flags;
	uint8_t base_high;		// The last 8 bits of the base.

}gdt_entry_t;

typedef struct gdt_ptr_struct_t
{
	uint16_t limit;			// the size of the gdt table MINUS one (this is the last valid index of our table)
	uint32_t base;			// The address of the first gdt_entry_t struct.

}gdt_ptr_t;

#pragma pack(pop)

extern gdt_entry_t gdt_entries[3];

void gdt_set_gate(gdt_entry_t* gdt_base, uint16_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
void gdtr_install(physical_addr base, uint32_t count, gdt_ptr_t* gdtr);

void gdt_print_gate(gdt_entry_t* gdt_base, uint16_t num);

#endif