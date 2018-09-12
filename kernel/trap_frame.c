#include <trap_frame.h>
#include <utility.h>
#include <gdt.h>
#include <debug.h>

void trap_frame_init(trap_frame_t* frame, virtual_addr_t entry_point, virtual_addr_t stack_top)
{
    memset(frame, 0, sizeof(trap_frame_t));

    // ! remove this line -- test purposes
    frame->ebx = 0xDEADBABA;
    frame->eax = 0x1BADB002;
    frame->ecx = 0x2BADB002;
    frame->edx = 0x3BADB002;


    frame->cs = GDT_USER_CS;
    frame->ds = frame->es = frame->fs = frame->gs = frame->ss = GDT_USER_DS;

    frame->eip = entry_point;
    frame->useresp = stack_top;
    frame->flags = get_flags() | 0x200;     // make sure interrupts are enabled
}

void trap_frame_print(trap_frame_t* frame)
{
    printfln("cs: %h ss: %h, ds: %h, gs: %h\neax: %h, ebx: %h, ecx: %h\neip: %h, u_esp: %h, flags: %h", 
                frame->cs, frame->ss, frame->ds, frame->gs,
                frame->eax, frame->ebx, frame->ecx,
                frame->eip, frame->useresp, frame->flags);
}