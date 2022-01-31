
MEMORY
{
    rom  (rx)  : ORIGIN = 0x08000000, LENGTH = 16K
    app1rom (rx) : ORIGIN = 0x8004000, LENGTH = 55k
    app2rom (rx) : ORIGIN = 0x8011C00, LENGTH = 55K
    nvsrom (rx) : ORIGIN = 0x0801F800, LENGTH = 2K
    ram (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}

__bootrom_start__ = ORIGIN(rom);
__bootrom_size__ = LENGTH(rom);
__app1rom_start__ = ORIGIN(app1rom);
__app1rom_size__ = LENGTH(app1rom);
__app2rom_start__ = ORIGIN(app2rom);
__app2rom_size__ = LENGTH(app2rom);
__nvsrom_start__ = ORIGIN(nvsrom);
__nvsrom_size__ = LENGTH(nvsrom);

/* Include the common ld script. */
INCLUDE cortex-m-generic.ld
