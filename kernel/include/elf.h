#ifndef ELF_H_21112018
#define ELF_H_21112018

/* Definitions for ELF-32 file format */

#include <types.h>

typedef uint32_t elf32_addr_t;
typedef uint16_t elf32_half_t;
typedef uint32_t elf32_off_t;
typedef int32_t elf32_sword_t;
typedef uint32_t elf32_word_t;

#define ELFMAG0		0x7f
#define ELFMAG1		'E'
#define ELFMAG2		'L'
#define ELFMAG3		'F'

/* Holds the indices into the elf identification field of useful stuff */
enum ELF32_ID_INDICES
{
	EI_MAG0,				// elf magic field 0
	EI_MAG1,				// elf magic field 1
	EI_MAG2,				// elf magic field 2
	EI_MAG3,				// elf magic field 3
	EI_CLASS,				// elf file class 
	EI_DATA,				// elf data encoding (little-big endian)
	EI_VERSION,				// elf file version
	EI_PAD,					// begin of PAD bytes
	EI_NIDENT	= 16		// size of e_ident array field
};

enum ELF32_FILE_CLASS
{
	ELFCLASSNONE,			// invalid class
	ELFCLASS32,				// 32-bit object
	ELFCLASS64				// 64-bit object
};

enum ELF32_DATA_ENC
{
	ELFDATANONE,
	ELFDATA2LSB,
	ELFDATA2MSB
};

enum ELF32_TYPE
{
	ET_NONE,					// no file type
	ET_REL,						// relocatable type
	ET_EXEC,					// executable type
	ET_DYN,						// shared object
	ET_CORE,					// core file
	ET_LOPROC = 0xff00,			// processor-specific
	ET_HIPROC = 0xffff			// processor-specific
};

enum ELF32_MACHINE
{
	EM_NONE,
	EM_M32,
	EM_SPARC,
	EM_386,				// intel 386
	EM_68K,
	EM_88K,
	EM_860,
	EM_MIPS
};

/* elf reserved section header indices */
enum ELF32_SECTION_IDX
{
	SHN_UNDEF,
	SHN_LORESERVE	= 0xff00,
	SHN_LOPROC		= 0xff00,
	SHN_HIPROC		= 0xff1f,
	SHN_ABS			= 0xfff1,
	SHN_COMMON		= 0xfff2,
	SHN_HIRESERVE	= 0xffff
};

enum ELF32_SECTION_TYPE
{
	SHT_NULL,						// marks an inactive section
	SHT_PROGBITS,					// section holds program bits
	SHT_SYMTAB,						// section holds symbol table
	SHT_STRTAB,						// section holds string table
	SHT_RELA,						// section holds relocation entries
	SHT_HASH,						// section holds symbol hash table
	SHT_DYNAMIC,					// section holds information for dynamic linking
	SHT_NOTE,						// section holds file marks
	SHT_NOBITS,						// section occupies no in-file space, but has the properties of SHT_PROGBITS
	SHT_REL,						// section holds relocation entries
	SHT_SHLIB,						// reserved
	SHT_DYNSYM,						// section holds symbol table
	SHT_LOPROC		= 0x70000000,
	SHT_HIPROC		= 0x7fffffff,
	SHT_LOUSER		= 0x80000000,
	SHT_HI_USER		= 0xffffffff
};

/* section flag bits. When set their properties apply to the section-defined memory region */
enum ELF32_SECTION_FLAGS
{
	SHF_WRITE		= 1,				// section contains writable data
	SHF_ALLOC		= 2,				// section occupies memory during execution
	SFH_EXECINSTR	= 4,				// section contains executable code
	SHF_MASKPROC	= 0xf0000000		// reserved
};

enum ELF32_PROGRAM_TYPE
{
	PT_NULL,							// marks an inactive segment
	PT_LOAD,							// loadable segment
	PT_DYNAMIC,							// specifies dynamic linking information
	PT_INTERP,							// specifies path to invoke interpreter
	PT_NOTE,							// specifies path to auxiliary information
	PT_SHLIB,							// reserved
	PT_PHDR,							
	PT_LOPROC	= 0x70000000,
	PT_HIPROC	= 0x7fffffff
};

enum ELF32_PROGRAM_FLAGS
{
	PF_NONE			= 0,
	PF_EXEC			= 1,
	PF_WRITE		= 2,
	PF_READ			= 4,
	PF_MASKOS		= 0x0ff00000,
	PF_MASKPROC		= 0xf0000000
};

/* 32-bit architecture ELF main header */
typedef struct
{
	uint8_t 		e_magic[3];				// elf magic number
	uint8_t			e_arch;					// 32 or 64 bit
	uint8_t 		e_endian;				// endianness
	uint8_t			e_iversion;				// elf version
	uint8_t			e_abi;					// OS ABI
	uint8_t			e_padding[8];			
	elf32_half_t	e_type;					// identifies the elf file type
	elf32_half_t	e_machine;				// specifies the required architecture
	elf32_word_t	e_version;				// identifies the current version
	elf32_addr_t	e_entry;				// specifies the virtual address that the program begins
	elf32_off_t		e_phoff;				// specifies the program header table's file offset
	elf32_off_t		e_shoff;				// specifies the section header table's file offset
	elf32_word_t	e_flags;				// specifies processor specific flags
	elf32_half_t	e_ehsize;				// specifies the ELF header's size in bytes
	elf32_half_t	e_phentsize;			// specifies the program header table entry's size (all entries are of the same size)
	elf32_half_t	e_phnum;				// specifies the number of program header entries
	elf32_half_t	e_shentsize;			// specifies the section header table entry's size (all entries are of the same size)
	elf32_half_t	e_shnum;				// specifies the number of section header table entris
	elf32_half_t	e_shstrndx;				// specifies the index of the name string table
} elf32_ehdr_t;

/* ELF section header */
typedef struct
{
	elf32_word_t	sh_name;				// index to string table for the name of this section
	elf32_word_t	sh_type;				// specifies the section's contents and semantics
	elf32_word_t	sh_flags;				// describes section's attributes
	elf32_addr_t	sh_addr;				// address in memory to map section (if the section is not to be present in memory, this is zero)
	elf32_off_t		sh_offset;				// in-file offset of the section
	elf32_word_t	sh_size;				// section size in bytes (note SHT_NOBITS)
	elf32_word_t	sh_link;				 
	elf32_word_t	sh_info;				// extra information based to section type
	elf32_word_t	sh_addralign;			// section alignment constraint
	elf32_word_t	sh_entsize;				
} elf32_section_hdr_t;

/* ELF program header */
typedef struct
{
	elf32_word_t	p_type;					// specifies the segment's content type
	elf32_off_t		p_offset;				// in-file offset of the segment
	elf32_addr_t	p_vaddr;				// address in memory to map the segment
	elf32_addr_t	p_paddr;				// address in physical memory to map the segment (used only for non-virtual-memory systems)
	elf32_word_t	p_filesz;				// segment size in bytes in the file (may be zero)
	elf32_word_t	p_memsz;				// segment size in bytes in the memory image (may be zero)
	elf32_word_t	p_flags;				// specifies flags relevant to the segment
	elf32_word_t	p_align;				// segment alignment constraint
} elf32_program_hdr_t;


void elf_print_metadata(elf32_ehdr_t* hdr);

void elf_print_header(elf32_ehdr_t* hdr);

void elf_print_program_header(elf32_program_hdr_t* hdr);

error_t elf_load(elf32_ehdr_t* hdr);

error_t elf_load_program_header(elf32_ehdr_t* hdr, elf32_program_hdr_t* phdr);

#endif