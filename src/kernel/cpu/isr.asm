[extern isr_handler]

[global isr0]
[global isr1]
[global isr2]
[global isr3]
[global isr4]
[global isr5]
[global isr6]
[global isr7]
[global isr8]
[global isr9]
[global isr10]
[global isr11]
[global isr12]
[global isr13]
[global isr14]
[global isr16]
[global isr17]
[global isr18]
[global isr19]
[global isr20]
[global isr21]
[global isr28]
[global isr29]
[global isr30]

; divide by zero
isr0:
    cli
    push qword 0 ; dummy error code
    push qword 0 ; interrupt number

    jmp isr_common

; debug
isr1:
    cli
    push qword 0 ; dummy error code
    push qword 1 ; interrupt number

    jmp isr_common

; non-maskable interrupt
isr2:
    cli
    push qword 0 ; dummy error code
    push qword 2 ; interrupt number

    jmp isr_common

; breakpoint
isr3:
    cli
    push qword 0 ; dummy error code
    push qword 3 ; interrupt number
    
    jmp isr_common

; overflow
isr4:
    cli
    push qword 0 ; dummy error code
    push qword 4 ; interrupt number
    
    jmp isr_common

; bound range exceeded
isr5:
    cli
    push qword 0 ; dummy error code
    push qword 5 ; interrupt number
    
    jmp isr_common

; invalid opcode
isr6:
    cli
    push qword 0 ; dummy error code
    push qword 6 ; interrupt number

    jmp isr_common

; device not available
isr7:
    cli
    push qword 0 ; dummy error code
    push qword 7 ; interrupt number
    
    jmp isr_common

; double fault
isr8:
    cli
    push qword 8 ; interrupt number

    jmp isr_common

; coprocessor segment overrun
isr9:
    cli
    push qword 0 ; dummy error code
    push qword 9 ; interrupt number
    
    jmp isr_common

; invalid TSS
isr10:
    cli
    push qword 10 ; interrupt number

    jmp isr_common

; segment not present
isr11:
    cli
    push qword 11 ; interrupt number

    jmp isr_common

; stack segment fault
isr12:
    cli
    push qword 12 ; interrupt number

    jmp isr_common

; general protection fault
isr13:
    cli
    push qword 13 ; interrupt number
    
    jmp isr_common

; page fault
isr14:
    cli
    push qword 14 ; interrupt number

    jmp isr_common

; x87 floating-point exception
isr16:
    cli
    push qword 0 ; dummy error code
    push qword 16 ; interrupt number

    jmp isr_common

; alignment check
isr17:
    cli
    push qword 17 ; interrupt number

    jmp isr_common

; machine check
isr18:
    cli
    push qword 0 ; dummy error code
    push qword 18 ; interrupt number

    jmp isr_common

; SIMD floating-point exception

isr19:
    cli
    push qword 0 ; dummy error code
    push qword 19 ; interrupt number

    jmp isr_common

; virtualization exception
isr20:
    cli
    push qword 0 ; dummy error code
    push qword 20 ; interrupt number

    jmp isr_common

; control protection exception
isr21:
    cli
    push qword 21 ; interrupt number

    jmp isr_common

isr28:
    cli
    push qword 0 ; dummy error code
    push qword 28 ; interrupt number

    jmp isr_common

isr29:
    cli
    push qword 29 ; interrupt number

    jmp isr_common

isr30:
    cli
    push qword 30 ; interrupt number

    jmp isr_common

isr_common:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    
    mov ax, ds
    push ax
    mov ax, es
    push ax
    mov ax, fs
    push ax
    mov ax, gs
    push ax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov rdi, rsp
    call isr_handler

    pop ax
    mov gs, ax
    pop ax
    mov ax, fs
    pop ax
    mov ax, es
    pop ax
    mov ax, ds

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    sti

    iretq