#ifndef UTILITY_H_12022018
#define UTILITY_H_12022018

#include <stdint.h>
#include <stddef.h>

#define isdigit(arg) ( arg >= '0' && arg <= '9' ? 1 : 0 )
#define isalpha(arg) ( ((arg >= 'a' && arg <= 'z') || (arg >= 'A' && arg <= 'Z')) ? 1 : 0 )
#define isalnum(arg) ( (isdigit(arg) || isalpha(arg)) ? 1 : 0 )
#define isupper(arg) ( (arg >= 'A' && arg <= 'Z') ? 1 : 0 )
#define islower(arg) ( (arg >= 'a' && arg <= 'z') ? 1 : 0 )

#define isprint(arg) ( arg >= 0x1f && arg < 0x7f )

#define tolower(arg) (  isupper(arg) ? arg + 32 : arg )
#define toupper(arg) (  islower(arg) ? arg - 32 : arg )

#define min(a, b)	 ( a < b ? a : b )
#define max(a, b)	 ( a < b ? b : a )

extern char hexes[];
extern char alphabet[];

// takes an unsigned int and returns to buffer its string base-representation
void uitoa(uint32_t val, char* buffer, uint8_t base);
void itoa(int32_t val, char* buffer, uint8_t base);

// takes an unsigned int and returns its alpha representation
// 1 -> a | 2 -> b | 25 -> aa
void uitoalpha(uint32_t val, char* buffer);

void memset(void* base, uint8_t val, uint32_t length);
void memcpy(void* dest, void* source, uint32_t length);

uint32_t atoui(char* buffer);
uint32_t atoui_dec(char* buffer, uint16_t length);
uint32_t atoui_hex(char* buffer, uint16_t length);

uint32_t pow(uint32_t base, uint32_t exp);
uint32_t ceil_division(uint32_t value, uint32_t divisor);

int is_aligned(uint32_t value, uint32_t with);


#endif