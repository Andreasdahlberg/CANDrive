
MEMORY
{
    bootrom  (rx)  : ORIGIN = 0x08000000, LENGTH = 16K
    rom (rx) : ORIGIN = 0x8004000, LENGTH = 110k
    nvsrom (rx) : ORIGIN = 0x0801F800, LENGTH = 2K
    ram (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}

__bootrom_start__ = ORIGIN(bootrom);
__bootrom_size__ = LENGTH(bootrom);
__approm_start__ = ORIGIN(rom);
__approm_size__ = LENGTH(rom);
__nvsrom_start__ = ORIGIN(nvsrom);
__nvsrom_size__ = LENGTH(nvsrom);

/* Include the common ld script. */
INCLUDE cortex-m-generic.ld
