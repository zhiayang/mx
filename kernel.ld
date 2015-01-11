ENTRY(KernelBootstrap)
OUTPUT_FORMAT(elf64-x86-64)

SECTIONS
{
	Kernel_VMA = 0xFFFFFFFF80000000;
	. = Kernel_VMA;

	.text : ALIGN(0x1000)
	{
		*(.text*)
		*(.gnu.linkonce.t*)
		*(.rodata*)
		*(.gnu.linkonce.r*)

		StartConstructors = .;
		*(SORT(.Constructors*))
		EndConstructors = .;


		StartDestructors = .;
		*(SORT(.Destructors))
		EndDestructors = .;

	}

	.data : ALIGN(0x1000)
	{
		*(.data*)
		*(.gnu.linkonce.d*)
	}

	.bss : ALIGN(0x1000)
	{
		StartBSS = .;
		*(COMMON)
		*(.bss*)
		*(.gnu.linkonce.b*)
		EndBSS = .;
	}
	KernelEnd = .;
}
