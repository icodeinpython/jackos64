#include <cpu/idt.h>
#include <kernel.h>

uint8_t exception_stack[512] __attribute__((aligned(16)));

void (*isr_handlers[32])(void) = {
    isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7,
    isr8, isr9, isr10, isr11, isr12, isr13, isr14,
    NULL,  // 15
    isr16, isr17, isr18, isr19, isr20, isr21,
    NULL, NULL, NULL, NULL, NULL, NULL,
    isr28, isr29, isr30, NULL
};



void isr_handler(regs_t* regs) {
    if (regs->int_no < 31 && isr_handlers[regs->int_no]) {
        isr_handlers[regs->int_no]();
    } else {
        // Handle unknown exceptions
        // For now, just halt the system
        hcf();
    }
}

void init_exceptions() {
    for (int i = 0; i < 32; i++) {
        if (isr_handlers[i]) {
            idt_set_gate(i, (uint64_t)isr_handlers[i], 0x08, 0x8E, 0);
        }
    }
}