[global load_gdt]
[extern gdtr]

load_gdt:
    lgdt [gdtr]
    push qword 0x08 
    lea rax, [.reload_cs]
    push rax
    retfq
.reload_cs:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret