#include <trap_frame.h>
#include <utility.h>
#include <gdt.h>
#include <debug.h>

void trap_frame_init(trap_frame_t* frame, virtual_addr_t entry_point, virtual_addr_t stack_top)
{
    memset(frame, 0, sizeof(trap_frame_t));

    frame->cs = GDT_USER_CS;
    frame->ds = frame->es = frame->fs = frame->gs = GDT_USER_DS;

    frame->eip = entry_point;
    frame->esp = stack_top;
    frame->flags = 0x202;               // ? set the IF flag and a reserve bit
}

void trap_frame_print(trap_frame_t* frame)
{
    printfln("cs: %h ds: %h, gs: %h\neax: %h, ebx: %h, ecx: %h\neip: %h, esp: %h, flags: %h", 
                frame->cs, frame->ds, frame->gs,
                frame->eax, frame->ebx, frame->ecx,
                frame->eip, frame->esp, frame->flags);
}