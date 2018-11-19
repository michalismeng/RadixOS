#include <thread_sched.h>
#include <mem_manager_virt.h>
#include <gst.h>

// * private functions

// returns the first non empty queue, priority taken into account.
uint32_t scheduler_get_first_non_empty(thread_sched_t* scheduler)
{
    uint32_t result = NUMBER_PRIORITIES;

	for (uint32_t i = HIGHEST_PRIORITY; i < NUMBER_PRIORITIES; i++)
    {
		if (scheduler->ready_heads[i])
        {
            result = i;
            break;			
        }
    }

	return result;
}

// * public functions

void scheduler_init(thread_sched_t* scheduler)
{
    memset(scheduler, 0, sizeof(thread_sched_t));
}

void scheduler_current_start()
{
    thread_sched_t* scheduler = &get_cpu_storage(get_cpu_id)->scheduler;
    scheduler_schedule_thread(scheduler);
    scheduler_current_execute();
}

void scheduler_add_ready(thread_sched_t* scheduler, TCB* thread)
{
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

void scheduler_current_add_ready(TCB* thread)
{
    thread_sched_t* scheduler = &get_cpu_storage(get_cpu_id)->scheduler;
    scheduler_add_ready(scheduler, thread);
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

void scheduler_save_thread_old(thread_sched_t* scheduler, TCB* thread)
{
    virtual_addr_t frame_base = thread->kframe.kernel_esp - 16;          // subtract 16 since some registers are already pushed (see pushad in idtr.asm)

    if(thread->is_kernel)
        memcpy(&thread->kframe, frame_base, sizeof(trap_frame_kernel_t));
    else        
        memcpy(&thread->frame, frame_base, sizeof(trap_frame_t));        // copy register contents to the trap frame of the executing thread
}

void scheduler_save_thread(TCB* thread, trap_frame_t* current_frame)
{
    if(thread->is_kernel)
        memcpy(&thread->kframe, current_frame, sizeof(trap_frame_kernel_t));
    else        
        memcpy(&thread->frame, current_frame, sizeof(trap_frame_t));        // copy register contents to the trap frame of the executing thread
}


void scheduler_restore_thread(thread_sched_t* scheduler, TCB* thread)
{
    virtual_addr_t frame_base = thread->kframe.kernel_esp - 16;

    if(thread->is_kernel)
    {
        // copy register contents from the new thread back to the stack
        memcpy(frame_base, &thread->kframe, sizeof(trap_frame_kernel_t));
    }
    else
    {
        // copy register contents from the new thread back to the stack
        memcpy(frame_base, &thread->frame, sizeof(trap_frame_t));
    }
}

TCB* scheduler_remove_running(thread_sched_t* scheduler)
{
    TCB* temp = scheduler->current_thread;
    scheduler->current_thread = 0;

    return temp;
}

TCB* scheduler_current_remove_running()
{
    thread_sched_t* scheduler = &get_cpu_storage(get_cpu_id)->scheduler;
    return scheduler_remove_running(scheduler);
}

TCB* scheduler_evict_thread(thread_sched_t* scheduler, trap_frame_t* current_frame)
{
    if(scheduler->current_thread)
    {
        TCB* old_thread = scheduler_remove_running(scheduler);
        scheduler_save_thread(old_thread, current_frame);

        return old_thread;
    }

    return 0;
}

TCB* scheduler_preempt_thread(thread_sched_t* scheduler, trap_frame_t* current_frame)
{
    TCB* thread = scheduler_evict_thread(scheduler, current_frame);
    if(thread)
        scheduler_add_ready(scheduler, thread);

    return thread;
}

TCB* scheduler_schedule_thread(thread_sched_t* scheduler)
{
    // pick a new thread to schedule
    uint32_t q_index = scheduler_get_first_non_empty(scheduler);
    
    if(q_index >= NUMBER_PRIORITIES)
    { 
        printfln("processor: %u", get_cpu_id);
        PANIC("invalid scheduling queue received");
    }

    // load the new running thread
    TCB* to_run = scheduler_remove_ready(scheduler, q_index);
    scheduler->current_thread = to_run;

    return to_run;
}

TCB* scheduler_current_schedule_thread()
{
    thread_sched_t* scheduler = &get_cpu_storage(get_cpu_id)->scheduler;
    return scheduler_schedule_thread(scheduler);
}

void scheduler_current_execute()
{
    thread_sched_t* scheduler = &get_cpu_storage(get_cpu_id)->scheduler;

    // TODO: maybe we need to turn off interrupts instead of just taking the lock

    acquire_spinlock(&scheduler->lock);
    TCB* thread = scheduler->current_thread;
    release_spinlock(&scheduler->lock);

    if(thread == 0)
        PANIC("scheduler received null thread to execute");

    virt_mem_switch_directory(thread->parent->page_dir);
    scheduler_restore_thread(scheduler, thread);

    virtual_addr_t frame_base = thread->kframe.kernel_esp - 16;

    if(thread->is_kernel)
    {
        // TODO: current ss and GDT_SS_ENTRY(get_cpu_id) * 8 seems to be the same
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
        asm("movl %0, %%esp; \
            pop %%gs; \
            pop %%fs; \
            pop %%es; \
            pop %%ds; \
            popal; \
            add $8, %%esp; \
            iret"::"r"(get_cpu_stack - sizeof(trap_frame_t)):"%esp");   //TODO: get_cpu_stack - sizeof(trap_frame_t) == frame_base ?
    }
}

void scheduler_print(thread_sched_t* scheduler)
{
    acquire_spinlock(&scheduler->lock);

    printfln("currently running: %u", scheduler->current_thread);
    
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

    release_spinlock(&scheduler->lock);
}

TCB* get_current_thread()
{
    return get_cpu_storage(get_cpu_id)->scheduler.current_thread;
}