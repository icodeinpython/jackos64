CC = x86_64-elf-gcc
AR = x86_64-elf-ar
ASM = nasm
CFLAGS = -ffreestanding -O2 -Wall -Wextra -nostdinc -I../include/libk -I../include/kernel

C_SRC = $(wildcard *.c)
ASM_SRC = $(wildcard *.asm)
S_SRC= $(wildcard *.S)

OBJ = $(C_SRC:.c=.c.o) $(ASM_SRC:.asm=.asm.o) $(S_SRC:.S=.S.o)

libk.a: $(OBJ)
	$(AR) rcs $@ $^

%.c.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.asm.o: %.asm
	$(ASM) -f elf64 $< -o $@

%.S.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@