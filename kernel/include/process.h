#ifndef PROCESS_H_28082016
#define PROCESS_H_28082016

#include <types.h>
#include <utility.h>
#include <mem_manager_virt.h>
#include <vm_contract.h>
#include <trap_frame.h>

#define MAX_PROCESS_SLOTS ((uint16_t)-1 / 2)
#define MAX_THREAD_SLOTS ((uint16_t)-1)

// enum THREAD_STATE {
// 	THREAD_NONE,
// 	THREAD_SLEEP,			// task resides in the sleep queue until its count-down timer reaches zero. It is then enqueued in the ready queue.
// 	THREAD_READY,			// task resides in the ready queue where it waits to be scheduled to run.
// 	THREAD_RUNNING,			// task does not reside in any queue as it is currently running.
// 	THREAD_BLOCK			// task resides in the block queue as it has requested blocking I/O service and waits for it to finish.
// };

// enum THREAD_ATTRIBUTE {
// 	THREAD_ATTR_NONE,
// 	THREAD_KERNEL = 1,					// thread is solely kernel => it does not have a user counter-part
// 	THREAD_NONPREEMPT = 1 << 1,			// thread is non pre-emptible. This applies only to kernel threads. Preemption state may change during thread execution
// 	THREAD_UNINTERRUPTIBLE = 1 << 2,	// thread is uninterruptible on SIGNALS ! Interrupts affect this thread. (uninterruptible is not cli)
// };

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

	uint16_t tid;							// thread unique id that correspons to the thread's index in the table
    uint8_t is_kernel;                      // set when the thread runs in kernel space (and uses kframe instaed of frame)
    uint8_t exec_cpu;                       // cpu id that this thread executes on

	PCB* parent;							// parent process that created this thread.

    uint32_t priotity;                      // thread scheduling priority

	struct thread_control_block* prev;		// previous thread in the scheduling queue (ready - sleep - block)
	struct thread_control_block* next;		// next thread in the scheduling queue

	// uint32_t sleep_time;							// time in millis of thread sleep. Used for the sleep function
	// int32_t plus_priority;						// priority gained due to different factors such as waiting in the queues
	// int32_t base_priority;						// base priority given at the thread creation time

}TCB;

typedef struct process_control_block
{
	process_flags_t flags;							// process flags
	uint16_t pid;									// unique process id that correspons to the process' index in the process table
	physical_addr page_dir;							// physical address of the page directory
	struct process_control_block* parent;			// parent PCB that created us.

	uint8_t name[16];								// name of the process

	vm_contract_t memory_contract;					// process memory map

	TCB* threads;									// pointer to list of child threads of the process
}PCB;

// initialize process and thread structures - allocate static memory
void process_init();

// create a process statically (try to acquire the specified slot in the table)
PCB* process_create_static(PCB* parent, physical_addr pdbr, uint8_t name[16], uint16_t pid);

// create a process dynamically (find an empty slot in the table)
PCB* process_create(PCB* parent, physical_addr pdbr, uint8_t name[16]);

// create a thread statically (try to acquire the specified slot in the table)
TCB* thread_create_static(PCB* parent, virtual_addr_t entry_point, virtual_addr_t stack_top, uint32_t priority, uint16_t tid, uint8_t is_kernel, uint8_t exec_cpu);

// create a thread dynamically (find an empty slot in the table)
TCB* thread_create(PCB* parent, virtual_addr_t entry_point, virtual_addr_t stack_top, uint32_t priority, uint8_t is_kernel, uint8_t exec_cpu);


// TCB* thread_create(PCB* parent, uint32_t entry, virtual_addr_t stack_top, uint32_t stack_size, uint32_t priority);

// int32_t thread_get_priority(TCB* thread);

// uint32_t* thread_get_error(TCB* thread);

// int thread_is_preemptible(TCB* thread);

// // add a 4-byte value to the stack so the awaken thread can receive it (like args)
// // void thread_add_parameter(TCB* thread, uint32_t param);

// // returns the thread with the lowest priority between the two.
// TCB* thread_get_lower_priority(TCB* thread1, TCB* thread2);

#endif
