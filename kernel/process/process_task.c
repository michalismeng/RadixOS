#include <process/process_task.h>
#include <process/process.h>
#include <thread_sched.h>
#include <gst.h>
#include <ipc/ipc.h>

// TODO: Fix this function
PRIVATE error_t handle_fork(message_t* msg)
{
    TCB* source = (TCB*)get_mailbox(msg->src)->owner;
    PCB* proc = source->parent;

    // allocate a new process identical to the caller
    PCB* new_process = process_create(proc->parent, proc->address_space.p_page_directory, proc->name);
    // allocate a new thread identical to the caller
    TCB* new_thread = thread_create(new_process, source->frame.eip, source->frame.esp, source->priotity, source->is_kernel, source->exec_cpu);

    // do some more stuff
    scheduler_add_ready(new_thread);
}

PUBLIC void process_task_entry_point()
{
    for(;;)
    {
        message_t msg;
        receive_current(&msg);

        // do work as indicated by the message request
        switch(msg.func)
        {
            case TM_FORK: handle_fork(&msg); break;
        }
    }
}