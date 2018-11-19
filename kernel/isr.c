#include <isr.h>
#include <utility.h>
#include <lapic.h>
#include <gst.h>
#include <debug.h>
#include <ipc/ipc.h>
#include <ipc/cpu_messages.h>

isr_t isr_handlers[ISR_HANDLERS];

int32_t* syscall_handler(trap_frame_t* regs)
{
    if(regs->eax == 10)
    {
        printfln("service number = %u", regs->eax);
        printfln("user stack = %h", regs->esp);
        printfln("user ds %h, gs %h", regs->ds, regs->gs);
        printfln("return eip: %h", regs->eip);
        printfln("user ss %h\n", regs->ss);
        
        printfln("kernel stack = %h %h", regs->kernel_esp, get_stack());
        printfln("kernel ds = %h, gs = %h, ss = %h", get_ds(), get_gs(), get_ss());
        printfln("cpu enabled: %u", per_cpu_read(PER_CPU_OFFSET(enabled)));
        printfln("--------------------------");
    }
    else if (regs->eax == 11)
        regs->eax = 3;
    else if (regs->eax == 12)
        printfln("echo");
    else if (regs->eax == 13)
    {
        printfln("eip: %h", get_eip());
        // scheduler_reschedule_current();
    }
    else if (regs->eax == 14)
        printfln("service 14 called");
    else if(regs->eax == 20)
    {
        TCB* cur_thread = get_cpu_storage(0)->scheduler.current_thread;
        printfln("Thread %u of process %s is over", cur_thread->tid, cur_thread->parent->name);
        PANIC("");
    }
    else
        PANIC("Unimplemented syscall");
}

void test_handle(trap_frame_t* regs)
{
    message_t msg;
    receive(get_cpu_storage(get_cpu_id)->mailbox, &msg);

    // do work

    printfln("cpu message function: %u core: %u", msg.func, get_cpu_id);

    if(msg.func == CM_RESCHEDULE)
    {
        printfln("rescheduling core: %u", get_cpu_id);
        // save thread context
        thread_sched_t* scheduler = &get_cpu_storage(get_cpu_id)->scheduler;
        scheduler_preempt_thread(scheduler, regs);

        scheduler_schedule_thread(scheduler);

        acknowledge(&msg);
        lapic_send_eoi(get_gst()->lapic_base);

        scheduler_current_execute();

        PANIC("DO NOT REACH HERE");
    }
    else if(msg.func == CM_SEM_WAIT)
    {
        printfln("semaphore waiting thread: %u", get_current_thread()->tid);
        semaphore_t* sem = (semaphore_t*)msg.payload.msg_ptr1.ptr;
        thread_sched_t* scheduler = &get_cpu_storage(get_cpu_id)->scheduler;

        // we only evict as the thread will be added to the semaphore wait queue
        TCB* thread = scheduler_evict_thread(scheduler, regs);

        if(sem->waiting_tail == 0)
            sem->waiting_head = sem->waiting_tail = thread;
        else
        {
            sem->waiting_tail->next = thread;
            sem->waiting_tail = thread;
            thread->next = 0;
        }
        // release the lock that was previously held by the semaphore
        release_spinlock(&sem->lock);

        scheduler_schedule_thread(scheduler);

        acknowledge(&msg);
        lapic_send_eoi(get_gst()->lapic_base);

        scheduler_current_execute();
    }
    else if(msg.func == CM_AWAKEN_THREAD)
    {
        TCB* thread = (TCB*)msg.payload.msg_ptr1.ptr;
        thread_sched_t* scheduler = &get_cpu_storage(get_cpu_id)->scheduler;

        scheduler_add_ready(scheduler, thread);
        scheduler_preempt_thread(scheduler, regs);

        scheduler_schedule_thread(scheduler);

        acknowledge(&msg);
        lapic_send_eoi(get_gst()->lapic_base);

        scheduler_current_execute();
    }

    // if(msg.func == 15)      // thread needs to awaken
    // {
    //     printfln("current thread: %u", get_cpu_storage(get_cpu_id)->scheduler.current_thread->tid);
    //     scheduler_current_add_ready(msg.payload.custom_int);
    //     scheduler_current_schedule_thread();
    //     acknowledge(&msg);                          // ! this should be the last thing to do... (but interrupts are off so we may be saved... CHECK)
    //     printfln("here");
    //     PANIC("");
    //     scheduler_current_execute();
    //     printfln("hello");
    //     // execution never reaches here
    //     // printfln("current thread: %u", get_cpu_storage(get_cpu_id)->scheduler.current_thread->tid);
    // }

    acknowledge(&msg);
}

void isr_init()
{
	printfln("isr handlers: %h", isr_handlers);
	memset(isr_handlers, 0, ISR_HANDLERS * sizeof(isr_t));

    isr_register(0x80, syscall_handler);
    isr_register(100, test_handle);
}

void isr_handler(trap_frame_t* regs)
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

void irq_handler(trap_frame_t* regs)
{
	printfln("spurious interrupt");
	// TODO: Send PIC EOI
}

void acpi_irq_handler(trap_frame_t* regs)
{
	if (isr_handlers[regs->int_no] != 0)
	{
		isr_t handler = isr_handlers[regs->int_no];
		handler(regs);
	}
	else
		printfln("Hardware interrupt: %u", regs->int_no);

	lapic_send_eoi(get_gst()->lapic_base);
}

void isr_register(uint8_t n, isr_t handler)
{
    isr_handlers[n] = handler;
}