#include <thread_sched.h>
#include <mem_manager_virt.h>
#include <gst.h>

// * private functions

// returns the first non empty queue, priority taken into account.
uint32_t scheduler_get_first_non_empty(thread_sched_t* scheduler)
{
	for (uint32_t i = HIGHEST_PRIORITY; i < NUMBER_PRIORITIES; i++)
		if (scheduler->ready_heads[i])
			return i;

	return 0;
}

// * public functions

void scheduler_init(thread_sched_t* scheduler)
{
    memset(scheduler, 0, sizeof(thread_sched_t));
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

void scheduler_stop_running_thread(thread_sched_t* scheduler)
{
    // TODO: when using get_stack() we imply the scheduler of the current processor -- no need for argument
    virtual_addr_t stack_top = ceil_division(get_stack(), virt_mem_get_page_size()) * virt_mem_get_page_size();

    // copy register contents to the trap frame of the executing thread
    memcpy(&scheduler->current_thread->frame, (void*)(stack_top - sizeof(trap_frame_t)), sizeof(trap_frame_t));

    // send the executing thread to the back of the queue
    scheduler_add_ready(scheduler, scheduler->current_thread);

    scheduler->current_thread = 0;
}

void scheduler_run_thread(thread_sched_t* scheduler)
{
    // pick a new thread to schedule
    uint32_t q_index = scheduler_get_first_non_empty(scheduler);
    if(q_index >= NUMBER_PRIORITIES)
        PANIC("invalid scheduling queue received");

    TCB* to_run = scheduler_remove_ready(scheduler, q_index);
    scheduler->current_thread = to_run;

    // switch to the new directory
    virt_mem_switch_directory(to_run->parent->page_dir);

    
    if(to_run->is_kernel)
    {
        virtual_addr_t stack_top = ceil_division(to_run->kframe.kernel_esp, virt_mem_get_page_size()) * virt_mem_get_page_size();
        memcpy((void*)(stack_top - sizeof(trap_frame_kernel_t)), (void*)&to_run->kframe, sizeof(trap_frame_kernel_t));
    }
    else
    {
        virtual_addr_t stack_top = ceil_division(get_stack(), virt_mem_get_page_size()) * virt_mem_get_page_size();
        // copy register contents from the new thread back to the stack
        memcpy((void*)(stack_top - sizeof(trap_frame_t)), (void*)&to_run->frame, sizeof(trap_frame_t));
    }
}

// void scheduler_reschedule(thread_sched_t* scheduler)
// {

// }

void scheduler_reschedule_current()
{
    thread_sched_t* scheduler = &get_cpu_storage(0)->scheduler;
    scheduler_stop_running_thread(scheduler);
    scheduler_run_thread(scheduler);
}

// TCB* current_thread = 0;
// _thread_sched scheduler;
// list<PCB*> procs;

// uint32 in_critical_section = 0;

// extern volatile uint32 ticks;
// extern "C" uint32 frequency;

// // all thread scheduling methods should never request heap memory, in order for the locking mechanisms to work
// // only thread insert should allocate memory
// // every other action is just a node move between the queues

// void thread_execute(TCB t)
// {
// 	__asm
// 	{
// 		// get the new address space
// 		mov eax, t.parent
// 		mov eax, [eax]

// 		push eax
// 		push eax

// 		call vmmngr_switch_directory

// 		; make a good stack
// 		add esp, 8

// 		// restore stack where all thread data where saved
// 		mov esp, t.esp

// 		// and pop them
// 		pop gs
// 		pop fs
// 		pop es
// 		pop ds

// 		popad

// 		iretd
// 	}
// }

// void scheduler_decrease_sleep_time()
// {
// 	dl_list_node<TCB*>* ptr = SLEEP_QUEUE.head;
// 	dl_list_node<TCB*>* prev = 0;

// 	uint32 elapsed = 1000 / frequency;

// 	while (ptr != 0)
// 	{
// 		TCB* thread = ptr->data;
// 		if (elapsed > thread->sleep_time)
// 		{
// 			dl_list_node<TCB*>* node;
// 			ptr = ptr->next;

// 			if (prev == 0)
// 				node = dl_list_remove_front_node(&SLEEP_QUEUE);
// 			else
// 				node = dl_list_remove_node(&SLEEP_QUEUE, prev);

// 			thread->state = THREAD_READY;
// 			thread->sleep_time = 0;
// 			dl_list_insert_back_node(&READY_QUEUE(thread_get_priority(thread)), node);
// 		}
// 		else
// 		{
// 			thread->sleep_time -= elapsed;
// 			prev = ptr;
// 			ptr = ptr->next;
// 		}
// 	}
// }

// extern "C" __declspec(naked) void scheduler_interrupt()
// {
// 	// already pushed due to interrupt flags, cs, eip
// 	__asm
// 	{
// 		cli

// 		// increase the time ticks
// 		push eax
// 		mov eax, dword ptr[ticks]
// 		inc eax
// 		mov dword ptr[ticks], eax
// 		pop eax

// 		// start threading
// 		cmp current_thread, 0
// 		je no_tasks

// 		THREAD_SAVE_STATE

// 		call scheduler_thread_switch
// 		call scheduler_decrease_sleep_time

// 		mov eax, dword ptr[current_thread]	// deref current_thread
// 		mov eax, [eax + 8]					// deref parent of thread
// 		mov eax, [eax]						// get page dir

// 		push eax
// 		push eax

// 		call vmmngr_switch_directory

// 		// restore previous esp from the now changed current thread pointer

// 		mov eax, dword ptr[current_thread]
// 		mov esp, [eax]

// 		// restore registers saved at the time current task was preempted.These are not the segments above as esp has changed.

// 		pop gs
// 		pop fs
// 		pop es
// 		pop ds

// 		popad

// no_tasks:

// 		push eax

// 		mov al, 20h
// 		out 20h, al

// 		pop eax
// 		iretd
// 	}
// }

// // public functions

// void init_thread_scheduler()
// {
// 	for (uint32 i = 0; i < NUMBER_PRIORITIES; i++)
// 		dl_list_init(&scheduler.thread_queues[i]);

// 	dl_list_init(&scheduler.block_queue);
// 	dl_list_init(&scheduler.sleep_queue);
// }

// TCB_node* thread_get_current_node()
// {
// 	INT_OFF;
// 	auto result = dl_list_find_node(&READY_QUEUE(thread_get_priority(current_thread)), current_thread);
// 	INT_ON;

// 	return result;
// }

// TCB* thread_get_current()
// {
// 	return current_thread;
// }

// PCB* process_get_current()
// {
// 	return current_thread->parent;
// }

// void scheduler_thread_switch()
// {
// 	// if the current thread is in a critical section (critlock), do not change execution
// 	if (in_critical_section)
// 		return;

// 	// assertions:
// 	// thread's state is saved on its stack
// 	// esp points to neutral stack

// 	// remove current thread from the executing state if it is in the running state. (See block thread removal below using this function)
// 	if (current_thread->state == THREAD_STATE::THREAD_RUNNING)
// 	{
// 		current_thread->state = THREAD_STATE::THREAD_READY;
// 		dl_list_head_to_tail(&READY_QUEUE(thread_get_priority(current_thread)));
// 	}

// 	dl_list<TCB*>* to_execute_list = sched_get_first_non_empty();
// 	if (to_execute_list == 0)		// nothing was found. Serious problem
// 		PANIC("Error scheduler execute list is empty");

// 	// setup thread to execute
// 	TCB* to_execute = LIST_PEEK(to_execute_list);

// 	if (to_execute->state != THREAD_STATE::THREAD_READY)
// 	{
// 		serial_printf("thread at %h id: %u stack_base: %h", to_execute, to_execute->id, to_execute->stack_top);
// 		PANIC("Received non ready thread to execute");
// 	}

// 	to_execute->state = THREAD_STATE::THREAD_RUNNING;
// 	current_thread = to_execute;
// 	// all that remains is to switch context and jump to the thread's eip
// }

// void scheduler_start()
// {
// 	dl_list<TCB*>* queue = sched_get_first_non_empty();
// 	if (queue == 0)
// 		PANIC("No thread found to start");

// 	current_thread = LIST_PEEK(queue);
// 	current_thread->state = THREAD_STATE::THREAD_RUNNING;
// 	thread_execute(*current_thread);
// }

// TCB_node* thread_insert(TCB* thread)
// {
// 	//TODO: if current thread's priority is lees than the new one's, the current should be pre-empted (except if it is non preemptible)
// 	thread->state = THREAD_STATE::THREAD_READY;
// 	TCB_node* node = new dl_list_node<TCB*>{ thread };
// 	dl_list_insert_back_node(&READY_QUEUE(thread_get_priority(thread)), node);
// 	return node;
// }

// // TODO: this must be enriched and better written
// void thread_set_priority(TCB* thread, uint32 priority)
// {
// 	//INT_OFF;

// 	//list_node<TCB*>* temp = 0;

// 	//// if the thread is ready or running we must move it up in the ready queue... else the thread_notify will do it
// 	//if (thread->state == THREAD_READY || thread->state == THREAD_RUNNING)
// 	//	temp = list_remove_node(&READY_QUEUE(thread_get_priority(thread)), list_get_prev(&READY_QUEUE(thread_get_priority(thread)), thread));

// 	//thread->base_priority = priority;

// 	//if (temp != 0)
// 	//	list_insert_back_node(&READY_QUEUE(thread_get_priority(thread)), temp);

// 	//INT_ON;
// }

// TCB* thread_find(uint32 id)
// {
// 	// we need a cli environment as we search a common data structure
// 	INT_OFF;

// 	dl_list_node<TCB*>* temp = 0;

// 	for (uint32 i = HIGHEST_PRIORITY; i < NUMBER_PRIORITIES; i++)
// 	{
// 		temp = READY_QUEUE(i).head;

// 		while (temp != 0)
// 		{
// 			if (temp->data->id == id)
// 				break;

// 			temp = temp->next;
// 		}
// 	}

// 	INT_ON;

// 	if (temp == 0)
// 		return 0;

// 	return temp->data;
// }

// __declspec(naked) void thread_current_yield()
// {
// 	THREAD_INTERRUPT_FRAME;		// save flags

// 	INT_OFF;					// mask interrupts. (we do not need to unmask as the previously saved flag state is not aware of this malicious change).

// 	THREAD_SAVE_STATE;			// save stack and change to neutral stack

// 	scheduler_thread_switch();
// 	thread_execute(*current_thread);
// }

// __declspec(naked) void thread_block(TCB_node* thread)
// {
// 	// TODO: Perhaps we will need to disable interrupts throughout this function to control the reading of the thread's state
// 	_asm push ebp
// 	_asm mov ebp, esp	// create a new stack frame, but destroy it immediatelly before messing with th thread stack

// 	INT_OFF;  // cli to mess with common data structure

// 	if (thread->data->state == THREAD_STATE::THREAD_BLOCK)
// 	{
// 		serial_printf("thread: %u already blocked\n", thread->data->id);
// 		PANIC(""); // return
// 	}

// 	if (thread->data->state != THREAD_STATE::THREAD_RUNNING && thread->data->state != THREAD_STATE::THREAD_READY)
// 	{
// 		printfln("thread: %u", thread->data->state);
// 		PANIC("thread error: thread neither READY nor RUNNING");  // iretd. Thread is neither running nor ready-waiting
// 	}

// 	// if the thread is not running
// 	// we simply remove the thread and add it to the blocked queue
// 	if (thread->data->state != THREAD_STATE::THREAD_RUNNING)
// 	{
// 		thread->data->state = THREAD_BLOCK;
// 		dl_list_remove_node(&READY_QUEUE(thread_get_priority(thread->data)), thread);
// 		dl_list_insert_back_node(&BLOCK_QUEUE, thread); 

// 		_asm pop ebp
// 	}
// 	else  // we need to firstly save its state on the stack
// 	{
// 		thread->data->state = THREAD_BLOCK;

// 		if (thread->data == current_thread)
// 		{
// 			dl_list_remove_front_node(&READY_QUEUE(thread_get_priority(thread->data)));
// 			dl_list_insert_back_node(&BLOCK_QUEUE, thread);

// 			_asm pop ebp

// 			THREAD_INTERRUPT_FRAME;

// 			THREAD_SAVE_STATE;

// 			scheduler_thread_switch();
// 			thread_execute(*current_thread);
// 		}
// 		else
// 			PANIC("Strange. Blocking a running thread from a running thread, they are not equal, UP system");  // there are two STATE == RUNNING threads
// 	}

// 	INT_ON;
// 	_asm ret
// }

// __declspec(naked) void thread_sleep(TCB_node* thread, uint32 sleep_time)
// {
// 	_asm push ebp
// 	_asm mov ebp, esp

// 	INT_OFF;  // cli to mess with common data structure

// 	if (thread->data->state == THREAD_STATE::THREAD_SLEEP)
// 	{
// 		serial_printf("THREAD %u IS ALREADY SLEEPING", thread->data->id);
// 		PANIC("");
// 	}

// 	if (thread->data->state != THREAD_STATE::THREAD_RUNNING && thread->data->state != THREAD_STATE::THREAD_READY)
// 	{
// 		printfln("thread: %u", thread->data->state);
// 		PANIC("current thread error");  // iretd. Thread is neither running nor ready-waiting
// 	}

// 	thread->data->sleep_time = sleep_time;

// 	// if the thread is not running
// 	// we simply remove the thread and add it to the sleeping queue
// 	if (thread->data->state != THREAD_STATE::THREAD_RUNNING)
// 	{
// 		thread->data->state = THREAD_SLEEP;

// 		dl_list_remove_node(&READY_QUEUE(thread_get_priority(thread->data)), thread);
// 		dl_list_insert_back_node(&SLEEP_QUEUE, thread);

// 		_asm pop ebp
// 	}
// 	else  // we need to firstly save its state at the stack
// 	{
// 		if (thread->data == current_thread)
// 		{
// 			thread->data->state = THREAD_SLEEP;

// 			dl_list_remove_front_node(&READY_QUEUE(thread_get_priority(thread->data)));
// 			dl_list_insert_back_node(&SLEEP_QUEUE, thread);

// 			_asm pop ebp  // fix the stack to create the interrupt frame

// 			INT_ON;

// 			THREAD_INTERRUPT_FRAME;

// 			INT_OFF;

// 			THREAD_SAVE_STATE;

// 			scheduler_thread_switch();
// 			thread_execute(*current_thread);
// 		}
// 		else
// 			PANIC("Strange. Sleeping a running thread from a running thread, they are not equal, UP system");  // there are two STATE == RUNNING threads
// 	}

// 	INT_ON;
// 	_asm ret
// }

// void thread_notify(TCB_node* thread)
// {
// 	INT_OFF;

// 	// thread must be blocked
// 	if (thread->data->state != THREAD_STATE::THREAD_BLOCK)
// 	{
// 		serial_printf("thread %u not blocked\n", thread->data->id);
// 		INT_ON;
// 		return;
// 	}

// 	auto node = dl_list_remove_node(&BLOCK_QUEUE, thread);
// 	thread->data->state = THREAD_STATE::THREAD_READY;
// 	dl_list_insert_back_node(&READY_QUEUE(thread_get_priority(thread->data)), node);


// 	// preempt the low priority thread, if it is preemptible
// 	/*if (thread_is_preemptible(thread_get_current()) && 
// 		thread_get_lower_priority(thread, thread_get_current()) == thread_get_current())
// 		thread_current_yield();*/		

// 	//INT_ON;
// }

// void scheduler_print_queue(TCB_list& queue)
// {
// 	if (queue.count == 0)
// 		return;

// 	TCB_node* ptr = queue.head;

// 	while (ptr != 0)
// 	{
// 		serial_printf("Task: %h with address space at: %h and esp: %h, id %u\n", ptr->data->id, ptr->data->parent->page_dir, ptr->data->esp, ptr->data->id);
// 		ptr = ptr->next;
// 	}
// }

// extern "C" void scheduler_print_queues()
// {
// 	serial_printf("noraml queues\n");
// 	for (int i = 0; i < NUMBER_PRIORITIES; i++)
// 	{
// 		if (scheduler.thread_queues[i].count == 0) continue;

// 		serial_printf("queue: %u\n", i);
// 		scheduler_print_queue(READY_QUEUE(i));
// 	}

// 	serial_printf("block queue\n");
// 	scheduler_print_queue(BLOCK_QUEUE);
// }