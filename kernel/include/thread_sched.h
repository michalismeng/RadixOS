#ifndef THREAD_SCHED_27112016
#define THREAD_SCHED_27112016

#include <types.h>
#include <process.h>
#include <sync/spinlock.h>

#define NUMBER_PRIORITIES 8
#define HIGHEST_PRIORITY 0
#define LOWEST_PRIORITY ( NUMBER_PRIORITIES - 1 )

typedef struct thread_sched
{	
	TCB* ready_heads[NUMBER_PRIORITIES];		// multilevel priority feedback queues. 0 is the highest -> NUMBER_PRIORITIES - 1 is the lowest
	TCB* ready_tails[NUMBER_PRIORITIES];
	// spinlock_t ready_lock;						// lock for the ready queues

	spinlock_t* lock;							// master scheduler lock, used when rescheduling

	// TCB* block_head;
	// TCB* block_tail;
	// spinlock_t block_lock;						// lock for the block queue

	// TCB* sleep_head;
	// TCB* sleep_tail;
	// spinlock_t sleep_lock;						// lock for the sleep queue

    TCB* current_thread;                        // the currently executing thread
	// spinlock_t cur_thread_lock;
} thread_sched_t;

// initialize a thread scheduler
void scheduler_init(thread_sched_t* scheduler);

// starts scheduling on the current cpu
void scheduler_current_start();


// add a new thread to the scheduler
void scheduler_add_ready(thread_sched_t* scheduler, TCB* thread);

// add a new thread to the current core
void scheduler_current_add_ready(TCB* thread);

// removes the head of the ready queue indexed by 'q_index'
TCB* scheduler_remove_ready(thread_sched_t* scheduler, uint32_t q_index);

// executes the currently selected thread on the current core
void scheduler_current_execute();

// removes the currently executing thread of a processor and returns it
TCB* scheduler_remove_running(thread_sched_t* scheduler);

// removes the currently running thread from the current core
TCB* scheduler_current_remove_running();

// removes the currently running thread and save its context
TCB* scheduler_evict_thread(thread_sched_t* scheduler, trap_frame_t* current_frame);

// evicts the currently running thread and puts it back to the ready queue
TCB* scheduler_preempt_thread(thread_sched_t* scheduler, trap_frame_t* current_frame);

// sets a new thread to run and returns it
TCB* scheduler_schedule_thread(thread_sched_t* scheduler);

// sets a new thread to run on the current core and returns it
TCB* scheduler_current_schedule_thread();

// print the schduler queues
void scheduler_print(thread_sched_t* schduler);

// returns the currently executing thread of this core
TCB* get_current_thread();

#endif