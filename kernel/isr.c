#include <isr.h>
#include <utility.h>
#include <lapic.h>
#include <gst.h>
#include <debug.h>

isr_t isr_handlers[ISR_HANDLERS];

int32_t* syscall_handler(iregisters_t* regs)
{
    printfln("user_gs = %u", regs->eax);
    printfln("service number = %u", regs->ebx);
    printfln("user stack = %h", regs->ecx);
    printfln("kernel stack = %h", get_stack());

    uint32_t ds_seg = get_ds();
    uint32_t gs_seg = get_gs();

    printfln("kernel ds = %h, gs = %h", ds_seg, gs_seg);
    asm ("mov $15, %eax; mov %eax, %gs");
    // printfln("cpu id: %u", per_cpu_read(PER_CPU_OFFSET(lapic_count)));
}

void isr_init()
{
	printfln("isr handlers: %h", isr_handlers);
	memset(isr_handlers, 0, ISR_HANDLERS * sizeof(isr_t));

    isr_register(0x80, syscall_handler);
}

void isr_handler(iregisters_t regs)
{
	// if(per_cpu_read(PER_CPU_OFFSET(id)) == 1){
	// 	printfln("Processor 1, exception: %u at %h", regs.int_no, isr_handlers[regs.int_no]);
	// 	PANIC("");
	// }

	if (isr_handlers[regs.int_no] != 0)
	{
		isr_t handler = isr_handlers[regs.int_no];
		handler(&regs);
	}
	else {
		printfln("Unhandled exception %u", regs.int_no);
		PANIC("");
	}
}

void irq_handler(iregisters_t regs)
{
	printfln("spurious interrupt");
	// TODO: Send PIC EOI
}

void acpi_irq_handler(iregisters_t regs)
{
	if (isr_handlers[regs.int_no] != 0)
	{
		isr_t handler = isr_handlers[regs.int_no];
		handler(&regs);
	}
	// else
	// 	printfln("Hardware interrupt: %u", regs.int_no);

	lapic_send_eoi(get_gst()->lapic_base);
}

void isr_register(uint8_t n, isr_t handler)
{
    isr_handlers[n] = handler;
}