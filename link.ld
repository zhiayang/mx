ENTRY(Boot)
OUTPUT_FORMAT(elf64-x86-64)

SECTIONS
{
	. = 0x00200000;
	.text : ALIGN(0x1000)
	{
		*(.text)
		*(.gnu.linkonce.t*)
	}
	.rodata : ALIGN(0x1000)
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

	.data : ALIGN(0x1000)
	{
		*(.data)
		*(.gnu.linkonce.d*)
	}

	.bss : ALIGN(0x1000)
	{
		StartBSS = .;
		*(COMMON)
		*(.bss)
		*(.gnu.linkonce.b*)
		EndBSS = .;
	}
	KernelEnd = .;
}
