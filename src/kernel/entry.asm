extern enable_features

[BITS 64]
[section .text]
[global _start]
[extern main]
[extern hcf]
_start:
    mov dx, 0x10
    mov ds, dx
    mov es, dx
    mov fs, dx
    mov gs, dx
    mov ss, dx
    mov rsp, stack_top
    mov rbp, rsp

    push rax ; magic
    push rbx ; pointer to the boot info structure
    call enable_features
    pop rdi ; rdi = pointer to the boot info structure
    pop rsi ; rsi = magic
    call main
    jmp hcf
    jmp $

[section .bss]
stack_bottom:
    resb 0x4000 ; 16KB stack
stack_top: