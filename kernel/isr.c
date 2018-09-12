#include <isr.h>
#include <utility.h>
#include <lapic.h>
#include <gst.h>
#include <debug.h>

isr_t isr_handlers[ISR_HANDLERS];

int32_t* syscall_handler(iregisters_t* regs)
{
    if(regs->eax == 10)
    {
        printfln("service number = %u", regs->eax);
        printfln("user stack = %h", regs->useresp);
        printfln("user ds %h, gs %h", regs->ds, regs->gs);
        printfln("return eip: %h", regs->eip);
        printfln("user ss %h\n", regs->ss);
        
        printfln("kernel stack = %h %h", regs->esp, get_stack());
        printfln("kernel ds = %h, gs = %h", get_ds(), get_gs());
        printfln("cpu enabled: %u", per_cpu_read(PER_CPU_OFFSET(enabled)));
    }
    else if (regs->eax == 11)
        regs->eax = 5;
    else if (regs->eax == 12)
        printfln("echo");
    else if(regs->eax == 20)
        PANIC("Thread is over");
    else
        PANIC("Unimplemented syscall");
}

void isr_init()
{
	printfln("isr handlers: %h", isr_handlers);
	memset(isr_handlers, 0, ISR_HANDLERS * sizeof(isr_t));

    isr_register(0x80, syscall_handler);
}

void isr_handler(iregisters_t* regs)
{
	if (isr_handlers[regs->int_no] != 0)
	{
		isr_t handler = isr_handlers[regs->int_no];
		handler(regs);
	}
	else {
		printfln("Unhandled exception %u %h", regs->int_no, regs->eip);
		PANIC("");
    }
}

void irq_handler(iregisters_t regs)
{
	printfln("spurious interrupt");
	// TODO: Send PIC EOI
}

void acpi_irq_handler(iregisters_t* regs)
{
	if (isr_handlers[regs->int_no] != 0)
	{
		isr_t handler = isr_handlers[regs->int_no];
		handler(&regs);
	}
	else
		printfln("Hardware interrupt: %u", regs->int_no);

	lapic_send_eoi(get_gst()->lapic_base);
}

void isr_register(uint8_t n, isr_t handler)
{
    isr_handlers[n] = handler;
}