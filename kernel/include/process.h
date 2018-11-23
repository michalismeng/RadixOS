#ifndef PROCESS_H_28082016
#define PROCESS_H_28082016

#include <types.h>
#include <utility.h>
#include <mem_manager_virt.h>
#include <vm_contract.h>
#include <trap_frame.h>
#include <ipc/mailbox.h>
#include <sync/semaphore.h>

#define MAX_PROCESS_SLOTS ((uint16_t)-1 / 2)
#define MAX_THREAD_SLOTS ((uint16_t)-1)

#define KERNEL_PROCESS_SLOT 0

// definitions for process slot flags used to determine the process state.
typedef enum {
	PROCESS_RUNNABLE = 0,
	PROCESS_SLOT_EMPTY,
	PROCESS_NEW

} process_flags_t;

// definitions for thread slot flags used to determine the thread state.
typedef enum {
	THREAD_RUNNABLE = 0,
	THREAD_SLOT_EMPTY,
    THREAD_NEW

} thread_flags_t;

typedef struct process_control_block PCB;

typedef struct thread_control_block
{
	thread_flags_t flags;					// thread flags
    union {
	    trap_frame_t frame;					// frame of registers, used to continue right where the thread left
        trap_frame_kernel_t kframe;         // kernel frame if thread is kernel
    };

	tid_t tid;								// thread unique id that correspons to the thread's index in the table
    uint8_t is_kernel;                      // set when the thread runs in kernel space (and uses kframe instaed of frame)
    uint8_t exec_cpu;                       // cpu id that this thread executes on

	PCB* parent;							// parent process that created this thread.

    uint32_t priotity;                      // thread scheduling priority

	mailbox_t* mailbox;						// mailbox for the thread

	struct thread_control_block* prev;		// previous thread in the scheduling queue (ready - sleep - block)
	struct thread_control_block* next;		// next thread in the scheduling queue

	// uint32_t sleep_time;							// time in millis of thread sleep. Used for the sleep function
	// int32_t plus_priority;						// priority gained due to different factors such as waiting in the queues
	// int32_t base_priority;						// base priority given at the thread creation time

}TCB;

typedef struct process_control_block
{
	process_flags_t flags;							// process flags
	pid_t pid;										// unique process id that correspons to the process' index in the process table
	physical_addr page_dir;							// physical address of the page directory
	struct process_control_block* parent;			// parent PCB that created us.

	uint8_t name[16];								// name of the process

	vm_contract_t memory_contract;					// process memory map

	// TODO: Add children thread pointers (requires changing the thread struct to include more next_thread pointers)
}PCB;

// initialize process and thread structures - allocate static memory
void process_init();

// create a process statically (try to acquire the specified slot in the table)
PCB* process_create_static(PCB* parent, physical_addr pdbr, uint8_t name[16], pid_t pid);

// create a process dynamically (find an empty slot in the table)
PCB* process_create(PCB* parent, physical_addr pdbr, uint8_t name[16]);

// get a process by its slot id
PCB* get_process(pid_t pid);

// create a thread statically (try to acquire the specified slot in the table)
TCB* thread_create_static(PCB* parent, virtual_addr_t entry_point, virtual_addr_t stack_top, uint32_t priority, tid_t tid, uint8_t is_kernel, uint8_t exec_cpu);

// create a thread dynamically (find an empty slot in the table)
TCB* thread_create(PCB* parent, virtual_addr_t entry_point, virtual_addr_t stack_top, uint32_t priority, uint8_t is_kernel, uint8_t exec_cpu);

// get a thread by its slot id
TCB* get_thread(tid_t tid);

// allocates a mailbox for the given thread and returns it
mailbox_t* thread_alloc_mailbox(TCB* thread);

mailbox_t* thread_alloc_mailbox_static(TCB* thread, mid_t mid);


#endif
