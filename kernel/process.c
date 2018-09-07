#include <process.h>

// private data and functions

// process table.
// PCB process_slots[MAX_PROCESS_SLOTS];

// thread table
// TCB thread_slots[MAX_THREAD_SLOTS];

// void thread_setup_execution_stack(TCB* t, uint32_t entry)
// {
// 	trap_frame_t* f;
// 	t->esp -= sizeof(trap_frame_t);		// prepare esp for manual data push
// 	f = (trap_frame_t*)t->esp;

// 	/* manual setup of the thread's stack */
// 	f->flags = 0x202;		// IF set along with some ?reserved? bit
// 	f->cs = 0x8;
// 	f->ds = 0x10;
// 	f->eax = 0;
// 	f->ebp = 0;
// 	f->ebx = 0;
// 	f->ecx = 0;
// 	f->edi = 0;
// 	f->edx = 0;
// 	f->eip = entry;
// 	f->es = 0x10;
// 	f->esi = 0;
// 	f->esp = 0;
// 	f->fs = 0x10;
// 	f->gs = 0x10;
// }

// // public functions

// PCB* process_create(PCB* parent, pdirectory_t* pdir, uint32_t low_address, uint32_t high_address)
// {
// 	PCB* proc = (PCB*)malloc(sizeof(PCB));

// 	proc->id = ++lastID;
// 	proc->parent = parent;

// 	if (pdir == 0)
// 	{
// 		proc->page_dir = vmmngr_create_address_space();
// 		vmmngr_map_kernel_space(proc->page_dir);
// 	}
// 	else
// 		proc->page_dir = pdir;

// 	// queue_init(&proc->threads);
// 	// vm_contract_init(&proc->memory_contract, low_address, high_address);
// 	// init_local_file_table(&proc->lft, 10);

// 	return proc;
// }

// // stack top is the top-most exclusive (last_valid + 1) value for stack.
// TCB* thread_create(PCB* parent, uint32_t entry, virtual_addr_t stack_top, uint32_t stack_size, uint32_t priority)
// {
// 	//printfln("creating thread at: %h with id: %u", esp, lastID + 1);
// 	if (stack_top % vmmngr_get_page_size() != 0)
// 		PANIC("stack must be page-aligned");

// 	TCB temp;
// 	TCB* t;
// 	queue_insert(&parent->threads, temp);

// 	t = &parent->threads.tail->data;

// 	t->id = ++lastID;
// 	t->parent = parent;
// 	t->stack_top = stack_top;
// 	t->state = THREAD_STATE::THREAD_READY;
// 	t->base_priority = priority;
// 	t->plus_priority = 0;
// 	t->attribute = THREAD_ATTRIBUTE::THREAD_KERNEL;
// 	t->thread_lock = THREAD_LOCK_NONE;
// 	t->ss = 0x10;
// 	t->esp = stack_top;

// 	// queue_spsc_init(&t->exceptions, 10);
// 	// t->exception_lock = 0;

// 	// TODO: Replace the directory switches by a simple kernel page map
// 	pdirectory* old_dir = vmmngr_get_directory();
// 	vmmngr_switch_directory(parent->page_dir, (physical_addr)parent->page_dir);

// 	// add the error variable at the bottom of the stack
// 	// thread_add_parameter(t, 0);

// 	// copy the parameter list
// 	// va_list params;
// 	// va_start(params, param_count);

// 	// for (uint32_t i = 0; i < param_count; i++)
// 	// {
// 	// 	uint32_t arg = va_arg(params, uint32_t);
// 	// 	serial_printf("adding %h to stack\n", arg);
// 	// 	thread_add_parameter(t, arg);
// 	// }

// 	// va_end(params);

// 	// thread_add_parameter(t, 0);	// add the return address. The thread should never return there, but it should properly call its destroy method.

// 	// finally setup the execution variables in the stack
// 	thread_setup_execution_stack(t, entry);
// 	vmmngr_switch_directory(old_dir, (physical_addr)old_dir);

// 	return t;
// }

// int32_t thread_get_priority(TCB* thread)
// {
// 	return thread->base_priority + thread->plus_priority;
// }

// uint32_t* thread_get_error(TCB* thread)
// {
// 	return (uint32_t*)((char*)thread->stack_top - 4);
// }

// int thread_is_preemptible(TCB* thread)
// {
// 	return ((thread->attribute & THREAD_NONPREEMPT) != THREAD_NONPREEMPT);
// }

// void thread_add_parameter(TCB* thread, uint32_t param)
// {
// 	thread->esp -= 4;
// 	*((uint32_t*)thread->esp) = param;
// }

// TCB* thread_get_lower_priority(TCB* thread1, TCB* thread2)
// {
// 	// (highest priority is 0, lowest is 7)
// 	return thread_get_priority(thread1) > thread_get_priority(thread2) ? thread1 : thread2;
// }