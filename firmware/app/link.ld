OUTPUT_FORMAT(elf32-littlearm)
OUTPUT_ARCH(arm)

_estack = 0x20005000;

MEMORY {
    FLASH : ORIGIN = 0x08000000, LENGTH = 128K
    RAM : ORIGIN = 0x20000000, LENGTH = 20K
}

SECTIONS
{
    .text :
    {
        *(vectors)    /* Vector table */
        *(.text)      /* Program code */
    } > FLASH

    .rodata :
    {
        *(.rodata)    /* Read only data */
    }

    _DATA_ROM_START = .;

    .  = 0x20000000;  /* From 0x20000000 */

    _DATA_RAM_START = .;
    .data : AT(_DATA_ROM_START)
    {
        *(.data)      /* Data memory */
    }
    _DATA_RAM_END = .;

    _BSS_START = .;   /* Indicates where BSS section starts in RAM */
    .bss :
    {
        *(.bss)       /* Zero-filled run time allocate data memory */
    }
    _BSS_END = .;     /* Indicates where BSS section ends in RAM */
}
