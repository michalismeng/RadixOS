#ifndef IDT_H_17022018
#define IDT_H_17022018

#include <types.h>

enum IDT_FLAGS
{
    IDT_TRAP    = 5,            // ISR defines 32-bit trap gate
    IDT_INT     = 14,           // ISR defines 32-bit interrupt gate
    IDT_TASK    = 15,           // ISR defines 32-bit task_gate

    IDT_USER    = 3 << 5,       // ISR DPL is set to user
};

#pragma pack(push, 1)

typedef struct idt_entry_struct_t
{
    uint16_t base_low;		// the lower 16 bits of the base of the addres to execute when the interrupt fires
    uint16_t selector;		// the kernel segment selector
    uint8_t always0;
    uint8_t flags;			// flags
    uint16_t base_high;		// the higher 16 bits of the address to execute
} idt_entry_t;

typedef struct idt_ptr_struct_t
{
    uint16_t limit;			// the size of the idt table MINUS one again as gdt table
    uint32_t base;			// the addres of the first idt_entry_t struct
} idt_ptr_t;

#pragma pack(pop)

void idt_init();
void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags);
void idtr_install(idt_ptr_t* idtr);

#endif