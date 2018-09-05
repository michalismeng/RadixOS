#ifndef PROCESS_H_28082016
#define PROCESS_H_28082016

#include <types.h>
#include <utility.h>
#include <mem_manager_virt.h>

#define MAX_TASKS 3
#define MAX_PROCESSES 30000

// setups interrupt frame for the thread (flags - cs - eip) based on the function's return eip
// assumes that function is naked
// #define THREAD_INTERRUPT_FRAME \
// _asm	pushfd	\
// _asm	push cs	\
// _asm	push eax \
// _asm	mov eax, dword ptr[esp + 12]	\
// _asm	xchg eax, dword ptr[esp + 4]	\
// _asm	xchg eax, dword ptr[esp + 8]	\
// _asm	xchg eax, dword ptr[esp + 12]	\
// _asm	pop eax	\

// #define THREAD_SAVE_STATE \
// _asm	pushad \
// _asm	push ds \
// _asm	push es \
// _asm	push fs \
// _asm	push gs \
// _asm	mov eax, dword ptr[current_thread] \
// _asm	mov[eax], esp \
// _asm	mov esp, 0x90000

enum THREAD_STATE {
	THREAD_NONE,
	THREAD_SLEEP,			// task resides in the sleep queue until its count-down timer reaches zero. It is then enqueued in the ready queue.
	THREAD_READY,			// task resides in the ready queue where it waits to be scheduled to run.
	THREAD_RUNNING,			// task does not reside in any queue as it is currently running.
	THREAD_BLOCK			// task resides in the block queue as it has requested blocking I/O service and waits for it to finish.
};

enum THREAD_ATTRIBUTE {
	THREAD_ATTR_NONE,
	THREAD_KERNEL = 1,					// thread is solely kernel => it does not have a user counter-part
	THREAD_NONPREEMPT = 1 << 1,			// thread is non pre-emptible. This applies only to kernel threads. Preemption state may change during thread execution
	THREAD_UNINTERRUPTIBLE = 1 << 2,	// thread is uninterruptible on SIGNALS ! Interrupts affect this thread. (uninterruptible is not cli)
};

// definitions for process slot flags used to determine the process state.
// when 0, process can be scheduled
typedef enum PROCESS_FLAGS {
	PROCESS_RUNNABLE = 0,
	PROCESS_SLOT_EMPTY

}process_flags_t;


typedef struct process_control_block PCB;

// typedef struct thread_control_block
// {
// 	uint32_t esp;
// 	uint32_t ss;

// 	PCB* parent;									// parent process that created this thread.
// 	uint32_t sleep_time;							// time in millis of thread sleep. Used for the sleep function

// 	uint32_t id;									// thread unique id

// 	int32_t plus_priority;							// priority gained due to different factors such as waiting in the queues
// 	int32_t base_priority;							// base priority given at the thread creation time

// 	virtual_addr_t stack_top;						// address of the base of the thread's stack

// 	enum THREAD_STATE state;						// the current state of the thread
// 	enum THREAD_ATTRIBUTE attribute;				// thread's extra attribute info
// 	// uint32_t thread_lock;							// thread lock status

// 	struct thread_control_block* prev;
// 	struct thread_control_block* next;
// }TCB;

typedef struct process_control_block
{
	process_flags_t flags;							// process flags
	uint32_t pid;									// unique process id that correspons to the process' index in the process table
	physical_addr page_dir;							// physical address of the page directory
	struct process_control_block* parent;			// parent PCB that created us. PCB 0 has null parent

	uint8_t name[16];								// name of the process

	struct process_control_block* next_ready;		// link to next ready process in the ready queue
	struct process_control_block* prev_ready;		// link to previous ready process in the ready queue


											// TODO: Perhaps these members will be erased
	// uint32_t image_base;						// base of the image this task is running
	// uint32_t image_size;						// size of the image this task is running

	// local_file_table lft;					// local open file table

	// vm_contract memory_contract;			// virtual memory layout
	// spinlock contract_spinlock;				// virtual memory contract spinlock used for reading and writing

	// TCB* threads;									// pointer to list of child threads of the process
}PCB;


// process table
PCB process_slots[MAX_TASKS + MAX_PROCESSES];

// PCB* process_create(PCB* parent, pdirectory_t* pdir, uint32_t low_address, uint32_t high_address);
// TCB* thread_create(PCB* parent, uint32_t entry, virtual_addr_t stack_top, uint32_t stack_size, uint32_t priority);

// int32_t thread_get_priority(TCB* thread);

// uint32_t* thread_get_error(TCB* thread);

// int thread_is_preemptible(TCB* thread);

// // add a 4-byte value to the stack so the awaken thread can receive it (like args)
// // void thread_add_parameter(TCB* thread, uint32_t param);

// // returns the thread with the lowest priority between the two.
// TCB* thread_get_lower_priority(TCB* thread1, TCB* thread2);

#endif
