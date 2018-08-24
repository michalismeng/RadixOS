#include <isr.h>
#include <utility.h>
#include <lapic.h>
#include <gst.h>
#include <debug.h>

isr_t isr_handlers[ISR_HANDLERS];

void isr_init()
{
	printfln("isr handlers: %h", isr_handlers);
	memset(isr_handlers, 0, ISR_HANDLERS * sizeof(isr_t));
}

void isr_handler(iregisters_t regs)
{
	if(per_cpu_read(PER_CPU_OFFSET(id)) == 1){
		printfln("Processor 1, exception: %u at %h", regs.int_no, isr_handlers[regs.int_no]);
		PANIC("");
	}

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