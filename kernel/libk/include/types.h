#ifndef TYPES_H_12022018
#define TYPES_H_12022018

#include <stdint.h>

typedef uint32_t error_t;

typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef void   VOID;

typedef uint32_t virtual_addr_t;
typedef uint32_t physical_addr;

typedef uint16_t tid_t;
typedef uint16_t pid_t;

enum error_t
{
	ERROR_OK = 1,
	ERROR_OCCUR = 0
};

#define VIRT_MEM_PAGES_PER_TABLE 1024	// intel arch definitions
#define VIRT_MEM_PAGES_PER_DIR	1024

#define CHK_BIT(x, bit) ( ( (x) & (bit) ) == (bit) )

#define KB *1024
#define MB KB*1024
#define GB MB*1024

#define PTR *
#define REF &

#define INVALID_FD -1
#define MAX_IO (size_t)-1 - 1
#define INVALID_IO (size_t)-1

typedef uint8_t* va_list;

#define sz(arg_type) ( sizeof(arg_type) % 4 != 0 ? sizeof(arg_type) + 4 - sizeof(arg_type) % 4 : sizeof(arg_type) )

#define va_start(pointer, last_arg) (  pointer = ( (va_list)&last_arg + sz(last_arg) )  )
#define va_end(pointer) (  pointer = (va_list) 0x0  )
#define va_arg(pointer, arg_type) ( *(arg_type*)( (pointer += sz(arg_type)) - sz(arg_type) ) )

// dereference macros
#define deref8(x)  *(uint8_t*) (x)
#define deref16(x) *(uint16_t*)(x)
#define deref32(x) *(uint32_t*)(x)


#endif