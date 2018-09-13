#ifndef ISR_H_07032018
#define ISR_H_07032018

#include <types.h>
#include <trap_frame.h>

#define ISR_HANDLERS 256

// interrupt service routine definition. It is assumed that user space code was interrupted, hence we do not use kernel frame.
// be aware if/when changing ss-esp-kernel_esp registers 
typedef int32_t(*isr_t)(trap_frame_t* regs);

void isr_init();

// register raw interrupt handler that is executed on cpu interrupt
void isr_register(uint8_t n, isr_t handler);


// typedef void(*isr_bottom_t)(struct thread_exception pe);

// TODO: Decide on the architecture of deferring
// register bottom interrupt handler that is executed in the deferred context
// void register_bottom_interrupt_handler(uint8_t n, isr_bottom_t handler);

#endif