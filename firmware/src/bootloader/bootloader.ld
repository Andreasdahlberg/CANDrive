
MEMORY
{
    rom  (rx)  : ORIGIN = 0x08000000, LENGTH = 32K
    approm (rx) : ORIGIN = 0x8008000, LENGTH = 94k
    nvsrom (rx) : ORIGIN = 0x0801F800, LENGTH = 2K
    ram (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}

__bootrom_start__ = ORIGIN(rom);
__bootrom_size__ = LENGTH(rom);
__approm_start__ = ORIGIN(approm);
__approm_size__ = LENGTH(approm);
__nvsrom_start__ = ORIGIN(nvsrom);
__nvsrom_size__ = LENGTH(nvsrom);

/* Include the common ld script. */
INCLUDE cortex-m-generic.ld
