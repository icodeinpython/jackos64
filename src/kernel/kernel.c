#include "kernel.h"
#include "bootloader.h"

int __attribute__((noreturn)) main();
int main() {
    hcf();
}

void hcf() {
hcf:
    __asm__ volatile ("hlt");
    goto hcf;
}
