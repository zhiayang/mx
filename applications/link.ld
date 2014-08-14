ENTRY(_start)
OUTPUT_FORMAT(elf64-x86-64)

SECTIONS
{
	. = 0x40000000;

	.text :
	{
		*(.text)
		*(.gnu.linkonce.t*)
	}
	.rodata :
	{
		StartConstructors = .;
		*(SORT(.Constructors*))
		EndConstructors = .;

		StartDestructors = .;
		*(SORT(.Destructors))
		EndDestructors = .;

		*(.rodata*)
		*(.gnu.linkonce.r*)
	}

	.data :
	{
		*(.data)
		*(.gnu.linkonce.d*)
	}

	.bss :
	{
		*(COMMON)
		*(.bss)
		*(.gnu.linkonce.b*)
	}
}
