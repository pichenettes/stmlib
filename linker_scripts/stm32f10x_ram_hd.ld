/*
 * STM32F103R8T6
 */

MEMORY
{
	RORAM (rx)  : ORIGIN = 0x20000000, LENGTH = 48K
	RAM   (rwx) : ORIGIN = 0x2000c000, LENGTH = 16K
}

SECTIONS
{
	.text :
	{
		_start = .;
		KEEP(*(.isr_vector))
		*(.text*)
		*(.rodata*)
		_etext = .;
		. = ALIGN(4);
		_sidata = .;
	} > RORAM

	.data : AT (ADDR(.text) + SIZEOF(.text))
	{
		_sdata = .;
		*(.data*)
		_edata = .;
	} > RAM

	.bss :
	{
		_sstack = .;
		. = . + 512;
		_estack = .;
		_sbss = .;
		*(.bss*)
		*(COMMON)
		_ebss = .;
	} > RAM
}
