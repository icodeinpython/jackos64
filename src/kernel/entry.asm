[BITS 64]
[section .text]
[global _start]
[extern main]
[extern hcf]
_start:
    mov ax, 10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000
    call main
    jmp hcf
    jmp $