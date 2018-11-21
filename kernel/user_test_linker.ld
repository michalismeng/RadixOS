ENTRY(do_user)

SECTIONS
{
	. = 0x600000;

    .text : ALIGN(32)
	{
		*(.text)
	}

	.data : ALIGN(32)
	{
		*(.data)
		*(.bss)
	}
}
