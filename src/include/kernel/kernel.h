#pragma once

static inline void bochs_breakpoint() {
    __asm__ volatile ("xchg %bx, %bx");
}

void hcf() __attribute__((noreturn));
void bochs_breakpoint();

extern struct bootinfo bootinfo;