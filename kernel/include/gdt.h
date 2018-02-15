#ifndef GDT_H_10022018
#define GDT_H_10022018

#include <stdint.h>

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

struct idt_entry_struct
{
	uint16_t base_low;		// the lower 16 bits of the base of the addres to execute when the interrupt fires
	uint16_t selector;		// the kernel segment selector
	uint8_t always0;
	uint8_t flags;			// flags
	uint16_t base_high;		// the higher 16 bits of the address to execute
};

struct idt_ptr_struct
{
	uint16_t limit;			// the size of the idt table MINUS one again as gdt table
	uint32_t base;			// the addres of the first idt_entry_t struct
};

#pragma pack(pop)

void gdt_set_gate(uint16_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
void gdtr_install();

void gdt_print_gate(uint16_t num);

#endif