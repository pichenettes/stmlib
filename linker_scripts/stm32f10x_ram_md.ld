/*
 * STM32F103R8T6
 */

MEMORY
{
	FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 128K
	RORAM (rx)  : ORIGIN = 0x20000000, LENGTH = 10K
	RAM   (rwx) : ORIGIN = 0x20002800, LENGTH = 10K
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

	.flashtext (NOLOAD) :
	{
		*(.flashtext*)
	} > FLASH
}
