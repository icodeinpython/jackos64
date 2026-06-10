#pragma once
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint16_t offset_low;    
    uint16_t selector;      
    uint8_t  ist;           
    uint8_t  type_attr;     
    uint16_t offset_mid;    
    uint32_t offset_high;   
    uint32_t zero;          
} __attribute__((packed)) idt_entry_t;

typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) idtr_t;

extern idt_entry_t idt[256];
extern idtr_t idtr;

void idt_set_gate(uint8_t vector, uint64_t offset, uint16_t selector, uint8_t type_attr, uint8_t ist);
void idt_init();

extern void load_idt();

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr28();
extern void isr29();
extern void isr30();

extern void (*isr_handlers[32])(void);

typedef struct {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint8_t int_no, err_code;
    uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((packed)) regs_t;

extern uint8_t exception_stack[512] __attribute__((aligned(16)));

void init_exceptions();