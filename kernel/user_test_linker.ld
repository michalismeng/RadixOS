ENTRY(do_user)

SECTIONS
{
	. = 4M;

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
