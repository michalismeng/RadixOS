#include <thread_sched.h>
#include <mem_manager_virt.h>
#include <gst.h>

// * private functions

// returns the first non empty queue, priority taken into account.
uint32_t scheduler_get_first_non_empty(thread_sched_t* scheduler)
{
	for (uint32_t i = HIGHEST_PRIORITY; i < NUMBER_PRIORITIES; i++)
		if (scheduler->ready_heads[i])
			return i;

	return NUMBER_PRIORITIES;
}

// * public functions

void scheduler_init(thread_sched_t* scheduler)
{
    memset(scheduler, 0, sizeof(thread_sched_t));
}

void scheduler_start()
{
    thread_sched_t* scheduler = &get_cpu_storage(get_cpu_id)->scheduler;
    scheduler_run_thread(scheduler);

    // we execute here if the first thread to execute is a user thread
    asm("movl %0, %%esp; \
        pop %%gs; \
        pop %%fs; \
        pop %%es; \
        pop %%ds; \
        popal; \
        add $8, %%esp; \
        iret"::"r"(get_cpu_stack - sizeof(trap_frame_t)):"%esp");
}

void scheduler_add_ready(TCB* thread)
{
    thread_sched_t* scheduler = &get_cpu_storage(thread->exec_cpu)->scheduler;
    uint32_t priority = thread->priotity;

    if(scheduler->ready_tails[priority] == 0)
    {
        scheduler->ready_tails[priority] = scheduler->ready_heads[priority] = thread;
        thread->next = thread->prev = 0;
    }
    else
    {
        scheduler->ready_tails[priority]->next = thread;
        thread->prev = scheduler->ready_tails[priority];
        thread->next = 0;
        scheduler->ready_tails[priority] = thread;
    }
}

TCB* scheduler_remove_ready(thread_sched_t* scheduler, uint32_t q_index)
{
    TCB* head = scheduler->ready_heads[q_index];
    scheduler->ready_heads[q_index] = head->next;

    if(scheduler->ready_heads[q_index] == 0)
        scheduler->ready_heads[q_index] = scheduler->ready_tails[q_index] = 0;
    else
        head->next->prev = 0;

    head->next = head->prev = 0;
    return head;
}

void scheduler_stop_running_thread(thread_sched_t* scheduler)
{
    virtual_addr_t frame_base = scheduler->current_thread->kframe.kernel_esp - 16;          // subtract 16 since some registers are already pushed (see pushad in idtr.asm)

    if(scheduler->current_thread->is_kernel)
        memcpy(&scheduler->current_thread->kframe, frame_base, sizeof(trap_frame_kernel_t));
    else        
        memcpy(&scheduler->current_thread->frame, frame_base, sizeof(trap_frame_t));        // copy register contents to the trap frame of the executing thread

    // send the executing thread to the back of the queue
    scheduler_add_ready(scheduler->current_thread);

    scheduler->current_thread = 0;
}

TCB* scheduler_run_thread(thread_sched_t* scheduler)
{
    // pick a new thread to schedule
    uint32_t q_index = scheduler_get_first_non_empty(scheduler);
    if(q_index >= NUMBER_PRIORITIES)
        PANIC("invalid scheduling queue received");

    TCB* to_run = scheduler_remove_ready(scheduler, q_index);
    scheduler->current_thread = to_run;

    // switch to the new directory
    virt_mem_switch_directory(to_run->parent->page_dir);

    virtual_addr_t frame_base = to_run->kframe.kernel_esp - 16;
    if(to_run->is_kernel)
    {
        // copy register contents from the new thread back to the stack
        memcpy(frame_base, &to_run->kframe, sizeof(trap_frame_kernel_t));

        // we have to change stack (but if we change we cannot return from this function => do "hard" return)
        asm("movl %0, %%esp; \
        movl %1, %%eax; \
        mov %%ax, %%ss; \
        pop %%gs; \
        pop %%fs; \
        pop %%es; \
        pop %%ds; \
        popal; \
        add $8, %%esp; \
        iret"::"r"(frame_base), "r"(GDT_SS_ENTRY(get_cpu_id) * 8):"%esp");
    }
    else
    {
        // copy register contents from the new thread back to the stack
        memcpy(frame_base, &to_run->frame, sizeof(trap_frame_t));
    }

    return to_run;
}

void scheduler_reschedule(thread_sched_t* scheduler)
{
    scheduler_stop_running_thread(scheduler);
    scheduler_run_thread(scheduler);
}

void scheduler_reschedule_current()
{
    thread_sched_t* scheduler = &get_cpu_storage(get_cpu_id)->scheduler;
    scheduler_reschedule(scheduler);
}

void scheduler_print(thread_sched_t* scheduler)
{
    for (uint32_t i = HIGHEST_PRIORITY; i < NUMBER_PRIORITIES; i++)
    {
		if (scheduler->ready_heads[i])
        {
            printfln("priority: %u", i);
            for(TCB* temp = scheduler->ready_heads[i]; temp; temp = temp->next)
            {
                printfln("thread %u on cpu %u", temp->tid, temp->exec_cpu);
            }
        }

    }
}