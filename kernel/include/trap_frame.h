#ifndef TRAP_FRAME_H_28082018
#define TRAP_FRAME_H_28082018

#include <types.h>

#pragma pack(push, 1)
	// represents the stack right before a user thread is loaded. (switch between rings 0-3)
	typedef struct trap_frame_user
	{
        uint32_t gs, fs, es, ds;
        uint32_t edi, esi, ebp, kernel_esp, ebx, edx, ecx, eax;
        uint32_t int_no, err_code;
        uint32_t eip, cs, flags, esp, ss;
	}trap_frame_user_t;

    typedef trap_frame_user_t trap_frame_t;

    // represents the stack right before a kernel thread is loaded. (switch between rings 0-0)
    typedef struct trap_frame_kernel
    {
        uint32_t gs, fs, es, ds;
        uint32_t edi, esi, ebp, kernel_esp, ebx, edx, ecx, eax;
        uint32_t int_no, err_code;
        uint32_t eip, cs, flags;

    }trap_frame_kernel_t;

#pragma pack(pop, 1)

// initialize a user thread trap frame
void trap_frame_init_user(trap_frame_t* frame, virtual_addr_t entry_point, virtual_addr_t stack_top);

// initialize a kernel thread trap frame
void trap_frame_init_kernel(trap_frame_kernel_t* frame, virtual_addr_t entry_point, virtual_addr_t stack_top, uint32_t IF);

// prints the trap frame
void trap_frame_print(trap_frame_t* frame);

#endif