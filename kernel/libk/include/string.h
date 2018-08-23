#ifndef STRING_H_12022018
#define STRING_H_12022018

#include <stdint.h>
#include <stddef.h>
#include <utility.h>

//get the length of a string
size_t strlen(const char* ch);

// compares two strings:
//		-1 if first un-match of s1 is lower than s2
//		0 if equal
//		1 if first un-match of s1 is greater than s2
int strcmp(const char* s1, const char* s2);

// compares strings without taking into account letter case.
int strcmp_insensitive(const char* s1, const char* s2);

// copies source string to destination. destination must be capable of holding the source string.
void strcpy(char* destination, char* source);

// copies source string to destination. Checks if destination can hold the source string.
uint8_t strcpy_s(char* destination, size_t destsz, const char* source);

// returns a pointer to the first occurence of the 'c' character given
char* strchr(char* str, int c);

// transforms a string to uppercase and returns it
char* strupper(char* str);

#endif