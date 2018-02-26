#include <idt.h>
#include <utility.h>
#include <gst.h>
#include <lapic.h>

idt_entry_t idt_entries[256];
idt_ptr_t idtr;

extern void _flushIDT(void* idtr);

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void isr128();

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

extern void apic_irq0();
extern void apic_irq1();
extern void apic_irq2();
extern void apic_irq3();
extern void apic_irq4();
extern void apic_irq5();
extern void apic_irq6();
extern void apic_irq7();
extern void apic_irq8();
extern void apic_irq9();
extern void apic_irq10();
extern void apic_irq11();
extern void apic_irq12();
extern void apic_irq13();
extern void apic_irq14();
extern void apic_irq15();

void idt_init()
{
    memset(idt_entries, 0, 256 * sizeof(idt_entry_t));

    idt_set_gate(0, (uint32_t)isr0, 0x08, IDT_INT);
	idt_set_gate(1, (uint32_t)isr1, 0x08, IDT_INT);
	idt_set_gate(2, (uint32_t)isr2, 0x08, IDT_INT);
	idt_set_gate(3, (uint32_t)isr3, 0x08, IDT_INT);
	idt_set_gate(4, (uint32_t)isr4, 0x08, IDT_INT);
	idt_set_gate(5, (uint32_t)isr5, 0x08, IDT_INT);
	idt_set_gate(6, (uint32_t)isr6, 0x08, IDT_INT);
	idt_set_gate(7, (uint32_t)isr7, 0x08, IDT_INT);
	idt_set_gate(8, (uint32_t)isr8, 0x08, IDT_INT);
	idt_set_gate(9, (uint32_t)isr9, 0x08, IDT_INT);
	idt_set_gate(10, (uint32_t)isr10, 0x08, IDT_INT);
	idt_set_gate(11, (uint32_t)isr11, 0x08, IDT_INT);
	idt_set_gate(12, (uint32_t)isr12, 0x08, IDT_INT);
	idt_set_gate(13, (uint32_t)isr13, 0x08, IDT_INT);
	idt_set_gate(14, (uint32_t)isr14, 0x08, IDT_INT);
	idt_set_gate(15, (uint32_t)isr15, 0x08, IDT_INT);
	idt_set_gate(16, (uint32_t)isr16, 0x08, IDT_INT);
	idt_set_gate(17, (uint32_t)isr17, 0x08, IDT_INT);
	idt_set_gate(18, (uint32_t)isr18, 0x08, IDT_INT);
	idt_set_gate(19, (uint32_t)isr19, 0x08, IDT_INT);
	idt_set_gate(20, (uint32_t)isr20, 0x08, IDT_INT);
	idt_set_gate(21, (uint32_t)isr21, 0x08, IDT_INT);
	idt_set_gate(22, (uint32_t)isr22, 0x08, IDT_INT);
	idt_set_gate(23, (uint32_t)isr23, 0x08, IDT_INT);
	idt_set_gate(24, (uint32_t)isr24, 0x08, IDT_INT);
	idt_set_gate(25, (uint32_t)isr25, 0x08, IDT_INT);
	idt_set_gate(26, (uint32_t)isr26, 0x08, IDT_INT);
	idt_set_gate(27, (uint32_t)isr27, 0x08, IDT_INT);
	idt_set_gate(28, (uint32_t)isr28, 0x08, IDT_INT);
	idt_set_gate(29, (uint32_t)isr29, 0x08, IDT_INT);
	idt_set_gate(30, (uint32_t)isr30, 0x08, IDT_INT);
	idt_set_gate(31, (uint32_t)isr31, 0x08, IDT_INT);

	idt_set_gate(0x80, (uint32_t)isr128, 0x08, 0xEE);	// set attributes to 0xEE to make it usable by the userspace programs

	idt_set_gate(64, (uint32_t)apic_irq0, 0x08, IDT_INT);
	idt_set_gate(65, (uint32_t)apic_irq1, 0x08, IDT_INT);
	idt_set_gate(66, (uint32_t)apic_irq2, 0x08, IDT_INT);
	idt_set_gate(67, (uint32_t)apic_irq3, 0x08, IDT_INT);
	idt_set_gate(68, (uint32_t)apic_irq4, 0x08, IDT_INT);
	idt_set_gate(69, (uint32_t)apic_irq5, 0x08, IDT_INT);
	idt_set_gate(70, (uint32_t)apic_irq6, 0x08, IDT_INT);
	idt_set_gate(71, (uint32_t)apic_irq7, 0x08, IDT_INT);
	idt_set_gate(72, (uint32_t)apic_irq8, 0x08, IDT_INT);
	idt_set_gate(73, (uint32_t)apic_irq9, 0x08, IDT_INT);
	idt_set_gate(74, (uint32_t)apic_irq10, 0x08, IDT_INT);
	idt_set_gate(75, (uint32_t)apic_irq11, 0x08, IDT_INT);
	idt_set_gate(76, (uint32_t)apic_irq12, 0x08, IDT_INT);
	idt_set_gate(77, (uint32_t)apic_irq13, 0x08, IDT_INT);
	idt_set_gate(78, (uint32_t)apic_irq14, 0x08, IDT_INT);
	idt_set_gate(79, (uint32_t)apic_irq15, 0x08, IDT_INT);

	idt_set_gate(224, (uint32_t)irq0, 0x08, IDT_INT);
	idt_set_gate(225, (uint32_t)irq1, 0x08, IDT_INT);
	idt_set_gate(226, (uint32_t)irq2, 0x08, IDT_INT);
	idt_set_gate(227, (uint32_t)irq3, 0x08, IDT_INT);
	idt_set_gate(228, (uint32_t)irq4, 0x08, IDT_INT);
	idt_set_gate(229, (uint32_t)irq5, 0x08, IDT_INT);
	idt_set_gate(230, (uint32_t)irq6, 0x08, IDT_INT);
	idt_set_gate(231, (uint32_t)irq7, 0x08, IDT_INT);
	idt_set_gate(232, (uint32_t)irq8, 0x08, IDT_INT);
	idt_set_gate(233, (uint32_t)irq9, 0x08, IDT_INT);
	idt_set_gate(234, (uint32_t)irq10, 0x08, IDT_INT);
	idt_set_gate(235, (uint32_t)irq11, 0x08, IDT_INT);
	idt_set_gate(236, (uint32_t)irq12, 0x08, IDT_INT);
	idt_set_gate(237, (uint32_t)irq13, 0x08, IDT_INT);
	idt_set_gate(238, (uint32_t)irq14, 0x08, IDT_INT);
	idt_set_gate(239, (uint32_t)irq15, 0x08, IDT_INT);
}

void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags)
{
	idt_entries[num].base_low = base & 0x0000FFFF;
	idt_entries[num].base_high = (base >> 16) & 0xFFFF;	// and with FFFF for safety. >> operator fills 0 in

	idt_entries[num].selector = selector;
	idt_entries[num].always0 = 0;
	idt_entries[num].flags = flags | (1 << 7);          // OR flags with the present bit
}

void idtr_install()
{
	idtr.base = (uint32_t)idt_entries;
	idtr.limit = 256 * sizeof(idt_entry_t) - 1;

	_flushIDT(&idtr);
}

void isr_handler(registers_t regs)
{
	printfln("isr %u", regs.int_no);
	//lapic_send_eoi(lapic_base);
}

volatile int pit_count = 0;

void irq_handler(registers_t regs)
{
	if(regs.int_no == 64)
		pit_count++;
	lapic_send_eoi(get_gst()->lapic_base);
}