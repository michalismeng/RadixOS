#ifndef KERNEL_DEFINITIONS_H_01052018
#define KERNEL_DEFINITIONS_H_01052018

#include <types.h>

/* Defines basic properties of the kernel */

/* kernel begin and end addresses as exported by the linker script */
extern uint8_t kernel_start;
extern uint8_t kernel_end;

#define KERNEL_START &kernel_start
#define KERNEL_END &kernel_end

#endif