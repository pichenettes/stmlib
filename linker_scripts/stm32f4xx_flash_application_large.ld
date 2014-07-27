/*
Default linker script for STM32F4xx_1024K_192K
*/

ENTRY(Reset_Handler)

_estack = 0x20020000;

_Min_Heap_Size = 0;
_Min_Stack_Size = 0x400;


MEMORY
{
  RAM (xrw)        : ORIGIN = 0x20000000, LENGTH = 128K
  CCMRAM (xrw)     : ORIGIN = 0x10000000, LENGTH = 64K
  FLASH (rx)       : ORIGIN = 0x08008000, LENGTH = 1M-32K
}

SECTIONS
{
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))            /* Startup code */
    . = ALIGN(4);
  } >FLASH

  .flashtext :
  {
    . = ALIGN(4);
    *(.flashtext)            /* Startup code */
    . = ALIGN(4);
  } >FLASH

  .text :
  {
    . = ALIGN(4);
    *(.text)                   /* remaining code */
    *(.text.*)                 /* remaining code */
    *(.rodata)                 /* read-only data (constants) */
    *(.rodata*)
    *(.glue_7)
    *(.glue_7t)
    KEEP (*(.init))
    KEEP (*(.fini))
    . = ALIGN(4);
     _etext = .;
     _sidata = _etext;
  } >FLASH
  
  .data : AT ( _sidata )
  {
    . = ALIGN(4);
    _sdata = . ;
    *(.ramtext*)
    . = ALIGN(4);
    *(.data*)
    *(.data.*)
    . = ALIGN(4);
     _edata = . ;
  } >RAM

  .bss :
  {
    . = ALIGN(4);
    _sbss = .;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
   _ebss = . ;
  } >RAM
  
  PROVIDE ( end = _ebss );
  PROVIDE ( _end = _ebss );
  
  ._user_heap_stack :
  {
    . = ALIGN(4);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(4);
  } >RAM
  
  /* CCM section, vars must be located here explicitly */
  /* Example: int foo __attribute__ ((section (".ccmdata"))); */
  .ccmdata (NOLOAD) :
  {
    . = ALIGN(4);
    *(.ccmdata)
    . = ALIGN(4);
  } >CCMRAM
 
  DISCARD :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }
}
