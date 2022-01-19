
MEMORY
{
    bootrom  (rx)  : ORIGIN = 0x08000000, LENGTH = 16K
    approm (rx) : ORIGIN = 0x08004000, LENGTH = 112K
    ram (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}

__bootrom_start__ = ORIGIN(bootrom);
__bootrom_size__ = LENGTH(bootrom);
__approm_start__ = ORIGIN(approm);
__approm_size__ = LENGTH(approm);
