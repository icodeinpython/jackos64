#include <stdint.h>

#include "cpu/gdt.h"

gdt_t gdt[3];
gdtr_t gdtr;

void gdt_set_descriptor(int index, uint8_t access, uint8_t gran) {
    gdt[index].base_low     = 0;
    gdt[index].base_middle  = 0;
    gdt[index].base_high    = 0;
    gdt[index].limit_low    = 0xFFFF; // Ignored by CPU but good practice to set

    gdt[index].access       = access;
    gdt[index].granularity  = (0x0F & 0xFFFF) | (gran & 0xF0); 
}


// initialize 64 bit kernel mode GDT
void gdt_init() {
    gdtr.limit = sizeof(gdt) - 1;
    gdtr.base = (uint64_t)&gdt;

    // Null descriptor
    gdt_set_descriptor(0, 0, 0);

    // Code segment descriptor
    gdt_set_descriptor(1, 0x9A, 0xA0);

    // Data segment descriptor
    gdt_set_descriptor(2, 0x92, 0xA0);

    load_gdt();
}