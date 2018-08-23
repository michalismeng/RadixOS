#ifndef DEBUG_H_12022018
#define DEBUG_H_12022018

#include <stdint.h>
#include <stddef.h>
#include <types.h>

void printf(char* fmt, ...);
void printfln(char* fmt, ...);

void PANIC(char* str);
void DEBUG(char* str);
void WARNING(char* str);

// when expr != 0, the function halts the system
void ASSERT(int expr);

#endif