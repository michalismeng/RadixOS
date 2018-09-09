#ifndef IREGS_H_07032018
#define IREGS_H_07032018

#include <stdint.h>

// Defines the registers found in the interrupt context
typedef struct iregisters_struct_t
{
    uint32_t ds, es, fs, gs;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;

} iregisters_t;

#endif