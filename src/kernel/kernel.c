#include "kernel.h"
#include "bootloader.h"
#include "cpu/gdt.h"
#include "cpu/idt.h"
#include "drivers/serial.h"
#include "mem/mem.h"
#include "drivers/screen.h"

#include <string.h>
#include <stdlib.h>

struct bootinfo bootinfo;


#include <stdio.h>

int __attribute__((noreturn)) main(struct bootinfo*, uint32_t);
int main(struct bootinfo* __bootinfo, uint32_t magic) {
    serial_init();
    puts_serial("Hello, World!\n");
    printf_serial("Bootloader magic: %#x\n", magic);
    if (magic != MAGIC) {
        hcf();
    }
    if (__bootinfo == NULL) {
        hcf();
    }
    if (__bootinfo->magic != MAGIC) {
        hcf();
    }
    copy_bootinfo(&bootinfo, __bootinfo);
    gdt_init();
    idt_init();
    init_exceptions();


    mem_init();

    initScreen();

    printf("Welcome to JACKOS!\n");


    hcf();
}

void hcf() {
hcf:
    __asm__ volatile ("hlt");
    goto hcf;
}
