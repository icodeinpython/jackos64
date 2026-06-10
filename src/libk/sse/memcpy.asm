[global memcpy]
[BITS 64]
memcpy:
    mov rax, rdi

.loop_avx:
    cmp rdx, 32
    jb .loop_tail

    vmovdqu ymm0, [rsi]
    vmovdqu [rdi], ymm0
    add rsi, 32
    add rdi, 32
    sub rdx, 32
    jmp .loop_avx

.loop_tail:
    cmp rdx, 0
    je .done

.tail_byte:
    movzx ecx, byte [rsi]
    mov [rdi], cl
    inc rsi
    inc rdi
    dec rdx
    jmp .loop_tail

.done:
    vzeroupper
    ret