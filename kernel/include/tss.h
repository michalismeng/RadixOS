#ifndef TSS_H_09092018
#define TSS_H_09092018

#include <stdint.h>
#include <gdt.h>

#pragma pack(push, 1)

typedef struct tss_entry
{
   uint32_t prev_tss;

   uint32_t esp0;       // stack pointer for kernel mode
   uint32_t ss0;        // stack segment for kernel mode

   uint32_t esp1;
   uint32_t ss1;
   uint32_t esp2;
   uint32_t ss2;
   uint32_t cr3;
   uint32_t eip;
   uint32_t eflags;
   uint32_t eax;
   uint32_t ecx;
   uint32_t edx;
   uint32_t ebx;
   uint32_t esp;
   uint32_t ebp;
   uint32_t esi;
   uint32_t edi;
   uint32_t es;         
   uint32_t cs;        
   uint32_t ss;        
   uint32_t ds;        
   uint32_t fs;       
   uint32_t gs;         
   uint32_t ldt;      
   uint16_t trap;
   uint16_t iomap_base;
} tss_entry_t;

#pragma pack(pop)

void tss_init_entry(gdt_entry_t* gdt_base, tss_entry_t* tss, uint16_t num);
void tss_install(uint16_t num);
void tss_set_kernel_stack(tss_entry_t* tss, uint32_t stack_top);

void _flushTSS(uint32_t selector);

#endif