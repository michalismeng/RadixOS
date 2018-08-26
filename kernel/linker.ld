ENTRY(_start)

SECTIONS
{
	. = 1M;

	/* Physical start address of the kernel initializer */
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

	. = 0xC0400000;

	/* Physical start address of the kernel */
	__kernel_physical_start = ADDR(.kernel_init) + SIZEOF(.kernel_init) + 4K - SIZEOF(.kernel_init) % 4K;

	/* Virtual start address of the kernel */
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

	/* Virtual end address of the kernel */
	kernel_end = .;
}
