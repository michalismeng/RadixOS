ENTRY(_start)

SECTIONS
{
	. = 1M;
	. += 4096;		/* Give a block space for the master physical memory manager block */

	__kernel_init_start = .;

	.kernel_init BLOCK(4K) : ALIGN(4K)
	{
		*(.multiboot)

		objs/boot.o(.text)
		objs/kernel_init.o(.text)
		
		objs/boot.o(.data)
		objs/kernel_init.o(.data)

		objs/boot.o(.bss)
		objs/kernel_init.o(.bss)
	}

	__kernel_init_end = .;

	/* Switch to virtual addresses */

	. = 0xC0000000;

	__kernel_physical_start = ADDR(.kernel_init) + SIZEOF(.kernel_init) + 4K - SIZEOF(.kernel_init) % 4K;
	kernel_start = .;

	.text BLOCK(4K) : AT( __kernel_physical_start)
	{
		*(.text)
	}

	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	}

	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}

	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}

	kernel_end = .;
}
