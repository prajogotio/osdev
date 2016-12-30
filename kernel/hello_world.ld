SECTIONS {
  . = 0xc0100000;
  .text : { *(.text) }
  .rodata : { *(.rodata) }
  .data : { *(.data) }
  .bss : {
    *(COMMON)
    *(.bss)
    }
}