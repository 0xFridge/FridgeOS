OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)

KERNEL_LOAD_ADDR = 0x1000;
ALIGNMENT = 4;

SECTIONS
{
  . = KERNEL_LOAD_ADDR;

  .text : ALIGN(ALIGNMENT)
  {
    *(.text*)
  }

  .rodata : ALIGN(ALIGNMENT)
  {
    *(.rodata*)
    *(.rdata*)
  }

  .data : ALIGN(ALIGNMENT)
  {
    *(.data*)
  }

  .bss : ALIGN(ALIGNMENT)
  {
    __bss_start = .;
    *(.bss*)
    *(COMMON)
    __bss_end = .;
  }

  /DISCARD/ :
  {
    *(.comment)
    *(.eh_frame)
    *(.note*)
  }
}
