OUTPUT_FORMAT("binary")

_kernel_virt_base = 0xE0000000;
_kernel_phys_base = 0x100000;

SECTIONS
{
	.start _kernel_phys_base : AT(0)
	{
		kstartup.o(.text)
		_send = .;
	}

	.text _kernel_virt_base + (_send - _kernel_phys_base) : AT(_send - _kernel_phys_base) 
	{
		*.o(.text*);
		*(.rodata*)
		_tend = .;
	}

	.data :
	{
		*(.data*)
	}

	_kernel_size = . - _kernel_virt_base;

	.bss :
	{
		*(.bss)
		*(COMMON)
	}

	_total_size = . - _kernel_virt_base;
}
