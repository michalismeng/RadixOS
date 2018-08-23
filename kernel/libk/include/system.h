#ifndef SYSTEM_H_13022018
#define SYSTEM_H_13022018

#include <stdint.h>

#define INT_OFF asm("cli");
#define INT_ON  asm("sti");

uint8_t inportb(uint16_t _port);
uint16_t inportw(uint16_t _port);
uint32_t inportl(uint16_t _port);

void outportb(uint16_t port, uint8_t data);
void outportw(uint16_t port, uint16_t data);
void outportl(uint16_t port, uint32_t data);

// read and write to a 32-bit register 
volatile uint32_t reg_readl(uint32_t base, uint16_t off);
void reg_writel(uint32_t base, uint16_t off, uint32_t val);

// void enable_write_protection();
// void disable_write_protection();

#endif