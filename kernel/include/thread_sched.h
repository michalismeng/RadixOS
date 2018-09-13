#ifndef THREAD_SCHED_27112016
#define THREAD_SCHED_27112016

#include <types.h>
#include <process.h>

#define NUMBER_PRIORITIES 8
#define HIGHEST_PRIORITY 0
#define LOWEST_PRIORITY ( NUMBER_PRIORITIES - 1 )

#define READY_QUEUE(x) scheduler.ready_queues[x]
#define BLOCK_QUEUE	   scheduler.block_queue
#define SLEEP_QUEUE	   scheduler.sleep_queue

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

// add a new thread to the scheduler
void scheduler_add_ready(thread_sched_t* scheduler, TCB* thread);

// removes the currently running thread
void scheduler_stop_running_thread(thread_sched_t* scheduler);

// sets a new thread to run
void scheduler_run_thread(thread_sched_t* scheduler);

// perform reschedule
// void scheduler_reschedule(thread_sched_t* scheduler);

// perform reschedule on the current processor
void scheduler_reschedule_current();


// extrnal default timer handler
// extern "C" void timer_callback(registers_t* regs);

// // executes a thread
// void thread_execute(TCB t);

// // initializes the thread scheduler
// void init_thread_scheduler();

// // puts the currently executing thread in the queues and finds another thread to execute
// void scheduler_thread_switch();

// // starts the scheduler
// void scheduler_start();

// // scheduler interrupt
// extern "C" void scheduler_interrupt();

// // returns the currently executing thread
// TCB* thread_get_current();

// TCB_node* thread_get_current_node();

// // returns the currently executing process (the parent of the executing thread)
// PCB* process_get_current();

// // finds a thread given its id
// TCB* thread_find(uint32 id);

// // blocks a thread
// void thread_block(TCB_node* thread);

// // awakens a blocked thread
// void thread_notify(TCB_node* thread);

// // put a thread to sleep
// void thread_sleep(TCB_node* thread, uint32 sleep_time);

// // yields the currently executing thread and reschedules. If this is the highest priority thread, it will be re-selected at rescheduling
// void thread_current_yield();

// // inserts a thread to its priority based ready queue
// TCB_node* thread_insert(TCB* thread);

// void thread_set_priority(TCB* thread, uint32 priority);

// extern "C" void scheduler_print_queues();

// void scheduler_print_queue(TCB_list& queue);

#endif