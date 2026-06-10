ENTRY(_start)


/* Define the physical and virtual offsets */
PHYS_BASE = 0x00100000; /* 1MB */
VIRT_BASE = 0xFFFFFFFF80000000; /* Higher-half kernel base address */

SECTIONS
{
    . = PHYS_BASE;
    _kernel_start_phys = .;

    /* 2. Higher-Half Kernel Sections
       We advance the Virtual Counter (.) to the higher-half address, 
       but we keep the Load Address (AT(...)) tracking the physical memory
       right after the bootstrap section. */
    . = VIRT_BASE + PHYS_BASE;
    _kernel_start_virt = .;

    .text : AT(ADDR(.text) - VIRT_BASE)
    {
        _code = .;
        *(.text*)
        *(.rodata*)
        . = ALIGN(4096);
    }

    .data : AT(ADDR(.data) - VIRT_BASE)
    {
        _data = .;
        *(.data*)
        . = ALIGN(4096);
    }

    .bss : AT(ADDR(.bss) - VIRT_BASE)
    {
        _bss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4096);
    }

    /* Define a symbol for the end of the kernel physical memory */
    . = ALIGN(4096);
    _kernel_end_phys = . - VIRT_BASE;
    _kernel_end_virt = .;

    /DISCARD/ : {
    *(.eh_frame)
    *(.note.*)
    }
}