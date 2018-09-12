#ifndef TRAP_FRAME_H_28082018
#define TRAP_FRAME_H_28082018

#include <types.h>

#pragma pack(push, 1)
	// represents the stack right before the thread is loaded. Must have this exact form.
	typedef struct trap_frame
	{
		// /* pushed by isr. */
		// uint32_t gs;
		// uint32_t fs;
		// uint32_t es;
		// uint32_t ds;
		// /* pushed by pusha. */
		// uint32_t eax;
		// uint32_t ebx;
		// uint32_t ecx;
		// uint32_t edx;
		// uint32_t esi;
		// uint32_t edi;
		// uint32_t esp;
		// uint32_t ebp;

        // uint32_t err_code;
        // uint32_t int_no;
		// /* pushed by cpu. */
		// uint32_t eip;
		// uint32_t cs;
		// uint32_t flags;
        // uint32_t useresp;
        // uint32_t ss;

        uint32_t gs, fs, es, ds;
        uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
        uint32_t int_no, err_code;
        uint32_t eip, cs, flags, useresp, ss;
	}trap_frame_t;

#pragma pack(pop, 1)

// initialize a user thread trap frame
void trap_frame_init(trap_frame_t* frame, virtual_addr_t entry_point, virtual_addr_t stack_top);

// prints the trap frame
void trap_frame_print(trap_frame_t* frame);

#endif