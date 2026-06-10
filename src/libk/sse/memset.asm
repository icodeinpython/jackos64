[global memset]
[BITS 64]

memset:
    ; 1. Broadcast the 8-bit char (RSI) to all 32 lanes of the YMM0 register
    movd        xmm0, esi           ; Move 8-bit value to xmm0 lower 32-bits
    vpbroadcastb ymm0, xmm0         ; Broadcast the byte to all 32 bytes of ymm0

    test rdx, rdx
    jz .done                        ; If count is zero, we're done

    ; 2. Initialize pointers and check size
    mov         rax, rdi            ; Return value: pointer to destination
    cmp         rdx, 32             ; If count < 32, jump to small byte-by-byte copies
    jb          .less_than_32

.loop_avx:
    ; 3. Write 32 bytes (256-bits) at a time
    vmovdqu     [rdi], ymm0         ; Unaligned 256-bit store (Use vmovdqa for strictly aligned data)
    add         rdi, 32
    sub         rdx, 32
    cmp         rdx, 32
    jae         .loop_avx

    cmp         rdx, 0
    je          .done

.less_than_32:
    ; 4. Handle remaining leftover bytes
    mov         [rdi], sil         ; Store the byte
    inc         rdi
    dec         rdx
    jnz         .less_than_32

.done:
    vzeroupper                      ; Clear upper AVX registers to avoid performance penalties
    ret
