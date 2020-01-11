SECTIONS {
    . = 0x00000000;
    .text : { head.o(.text)
              *(.text) 
            }
    .rodata ALIGN(4) : {*(.rodata*)} 
    .data   ALIGN(4) : { *(.data) }
    __bss_start__ = .;
    .bss ALIGN(4)  : { *(.bss)  *(COMMON) }
    __bss_end__ = .;
}

