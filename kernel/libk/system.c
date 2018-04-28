#include <system.h>

uint8_t inportb(uint16_t port)
{
   uint8_t ret;
   asm volatile ("inb %%dx,%%al":"=a" (ret):"d" (port));
   return ret;
}

uint16_t inportw(uint16_t port)
{
   uint16_t ret;
   asm volatile ("inw %%dx,%%ax":"=a" (ret):"d" (port));
   return ret;
}

uint32_t inportl(uint16_t port)
{
   uint32_t ret;
   asm volatile ("inl %%dx,%%eax":"=a" (ret):"d" (port));
   return ret;
}

void outportb(uint16_t port, uint8_t value)
{
   asm volatile ("outb %%al,%%dx": :"d" (port), "a" (value));
}

void outportw(uint16_t port, uint16_t value)
{
   asm volatile ("outw %%ax,%%dx": :"d" (port), "a" (value));
}

void outportl(uint16_t port, uint32_t value)
{
   asm volatile ("outl %%eax,%%dx": :"d" (port), "a" (value));
}

volatile uint32_t reg_readl(uint32_t base, uint16_t off)
{
    uint32_t volatile* addr = base + off;
    return *addr;
}

void reg_writel(uint32_t base, uint16_t off, uint32_t val)
{
    uint32_t volatile* addr = base + off;
    *addr = val;
}

// void enable_write_protection()
// {
// 	_asm {
// 		mov eax, cr0
// 		or eax, 0x10000
// 		mov cr0, eax
// 	}
// }

// void disable_write_protection()
// {
// 	_asm {
// 		mov eax, cr0
// 		and eax, ~0x10000
// 		mov cr0, eax
// 	}
// }