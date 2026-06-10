#include <stdint.h>
#include <string.h>

#include "kernel.h"
#include "cpu/idt.h"

idt_entry_t idt[256];
idtr_t idtr;

void idt_set_gate(uint8_t vector, uint64_t offset, uint16_t selector, uint8_t type_attr, uint8_t ist) {
    idt[vector].offset_low = offset & 0xFFFF;
    idt[vector].selector = selector;
    idt[vector].ist = ist & 0x7; // Only 3 bits for IST
    idt[vector].type_attr = type_attr;
    idt[vector].offset_mid = (offset >> 16) & 0xFFFF;
    idt[vector].offset_high = (offset >> 32) & 0xFFFFFFFF;
    idt[vector].zero = 0;
}

void idt_init() {
    idtr.base = (uint64_t)idt;
    idtr.limit = sizeof(idt) - 1;

    memset(&idt[0], 0, sizeof(idt));
    load_idt();
}