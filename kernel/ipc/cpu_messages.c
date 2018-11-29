#include <ipc/cpu_messages.h>
#include <ipc/message.h>
#include <thread_sched.h>
#include <gst.h>

// private functions

PRIVATE void handle_reschedule(message_t* msg, trap_frame_t* frame)
{
    printfln("rescheduling core: %u", get_cpu_id);
    // save thread context
    thread_sched_t* scheduler = &get_cpu_storage(get_cpu_id)->scheduler;
    scheduler_preempt_thread(scheduler, frame);

    scheduler_schedule_thread(scheduler);

    acknowledge(msg);
    lapic_send_eoi(get_gst()->lapic_base);

    scheduler_current_execute();

    PANIC("DO NOT REACH HERE");
}

PRIVATE void handle_sem_wait(message_t* msg, trap_frame_t* frame)
{
    printfln("semaphore waiting thread: %u", get_current_thread()->tid);
    semaphore_t* sem = (semaphore_t*)msg->payload.msg_ptr1.ptr;
    thread_sched_t* scheduler = &get_cpu_storage(get_cpu_id)->scheduler;

    // we only evict as the thread will be added to the semaphore wait queue
    TCB* thread = scheduler_evict_thread(scheduler, frame);

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

    acknowledge(msg);
    lapic_send_eoi(get_gst()->lapic_base);

    scheduler_current_execute();
}

PRIVATE void handle_awaken(message_t* msg, trap_frame_t* frame)
{
    TCB* thread = (TCB*)msg->payload.msg_ptr1.ptr;
    thread_sched_t* scheduler = &get_cpu_storage(get_cpu_id)->scheduler;

    scheduler_add_ready(scheduler, thread);
    scheduler_preempt_thread(scheduler, frame);

    scheduler_schedule_thread(scheduler);

    acknowledge(msg);
    lapic_send_eoi(get_gst()->lapic_base);

    scheduler_current_execute();
}

// public functions

PUBLIC void cpu_message_handler(trap_frame_t* regs)
{
    message_t msg;
    receive(get_cpu_storage(get_cpu_id)->mailbox, &msg);

    // do work

    printfln("cpu message function: %u core: %u", msg.func, get_cpu_id);

    switch(msg.func)
    {
        case CM_RESCHEDULE:     handle_reschedule(&msg, regs); break;
        case CM_SEM_WAIT:       handle_sem_wait(&msg, regs); break;
        case CM_AWAKEN_THREAD:  handle_awaken(&msg, regs); break;
    }

    // when done acknowledge message (cpu-only messages require this) to release the message spinlock
    acknowledge(&msg);
}
