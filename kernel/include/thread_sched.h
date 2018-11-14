#ifndef THREAD_SCHED_27112016
#define THREAD_SCHED_27112016

#include <types.h>
#include <process.h>

#define NUMBER_PRIORITIES 8
#define HIGHEST_PRIORITY 0
#define LOWEST_PRIORITY ( NUMBER_PRIORITIES - 1 )

typedef struct thread_sched
{	
	TCB* ready_heads[NUMBER_PRIORITIES];		// multilevel priority feedback queues. 0 is the highest -> NUMBER_PRIORITIES - 1 is the lowest
	TCB* ready_tails[NUMBER_PRIORITIES];

	TCB* block_head;
	TCB* block_tail;

	TCB* sleep_head;
	TCB* sleep_tail;

    TCB* current_thread;                        // the currently executing thread
} thread_sched_t;

// initialize a thread scheduler
void scheduler_init(thread_sched_t* scheduler);

// starts scheduling on the current cpu
void scheduler_start();

// add a new thread to the scheduler
void scheduler_add_ready(TCB* thread);

// removes the currently running thread
void scheduler_stop_running_thread(thread_sched_t* scheduler);

// sets a new thread to run and returns it
TCB* scheduler_run_thread(thread_sched_t* scheduler);

// perform reschedule on the given scheduler (=> change cpu running thread)
void scheduler_reschedule(thread_sched_t* scheduler);

// perform reschedule on the current processor
void scheduler_reschedule_current();

// print the schduler queues
void scheduler_print(thread_sched_t* schduler);

#endif