#ifndef THREAD_SCHED_27112016
#define THREAD_SCHED_27112016

#include <types.h>
#include <process.h>
#include <spinlock.h>

#define NUMBER_PRIORITIES 8
#define HIGHEST_PRIORITY 0
#define LOWEST_PRIORITY ( NUMBER_PRIORITIES - 1 )

typedef struct thread_sched
{	
	TCB* ready_heads[NUMBER_PRIORITIES];		// multilevel priority feedback queues. 0 is the highest -> NUMBER_PRIORITIES - 1 is the lowest
	TCB* ready_tails[NUMBER_PRIORITIES];
	spinlock_t ready_lock;						// lock for the ready queues

	TCB* block_head;
	TCB* block_tail;
	spinlock_t block_lock;						// lock for the block queue

	TCB* sleep_head;
	TCB* sleep_tail;
	spinlock_t sleep_lock;						// lock for the sleep queue

    TCB* current_thread;                        // the currently executing thread
	spinlock_t cur_thread_lock;
} thread_sched_t;

// initialize a thread scheduler
void scheduler_init(thread_sched_t* scheduler);

// starts scheduling on the current cpu
void scheduler_start();

// add a new thread to the scheduler
void scheduler_add_ready(thread_sched_t* scheduler, TCB* thread);

// removes the currently running thread
void scheduler_stop_running_thread(thread_sched_t* scheduler);

// sets a new thread to run and returns it
TCB* scheduler_run_thread(thread_sched_t* scheduler);

// perform reschedule on the given scheduler (=> change cpu running thread)
void scheduler_reschedule(thread_sched_t* scheduler);

// perform reschedule on the current processor
void scheduler_reschedule_current();

// blocks the currently running thread of the current processor
void scheduler_block_running_thread();

// print the schduler queues
void scheduler_print(thread_sched_t* schduler);

#endif