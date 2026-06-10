[global enable_features]

enable_features:
    call check_sse
    call enable_sse
    call enable_avx

    ret


check_sse:
    mov eax, 1
    cpuid
    test edx, (1 << 25) ; Check SSE bit
    jz features_error
    ; SSE is supported, you can enable it here if needed

    ret

enable_sse:
    mov rax, cr0
    and ax, 0xFFFB  ; clear coprocessor emulation bit
    or ax, 0x2      ; set monitor coprocessor bit
    mov cr0, rax
    mov rax, cr4
    or rax, (3 << 9) ; set OSFXSR and OSXMMEEXCPT bits
    mov cr4, rax
    ret

features_error:
    cli
    hlt
    jmp $

enable_avx:
    mov rax, cr4
    or rax, (1 << 18) ; set OSXSAVE bit
    mov cr4, rax
    ; Check for CPUID support for AVX
    mov eax, 1
    cpuid
    and ecx, 0x10000000    ; Test bit 28 of ECX (AVX support)
    jz features_error             ; If 0, AVX is not supported

    ; Check if OS saves YMM registers
    mov eax, 7
    xor edx, edx
    xor ecx, ecx             ; XCR0 register
    xsetbv                 ; Write Extended Control Register 0
    ret