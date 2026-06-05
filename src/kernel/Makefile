CC = x86_64-elf-gcc
LD = x86_64-elf-ld
ASM = nasm
CFLAGS = -ffreestanding -O2 -Wall -Wextra -I ../include/kernel -isystem ../include/libk -mcmodel=kernel

C_SRC = $(shell find -name "*.c")
ASM_SRC = $(shell find -name "*.asm")
S_SRC = $(shell find -name "*.S")

OBJ = $(C_SRC:.c=.c.o) $(ASM_SRC:.asm=.asm.o) $(S_SRC:.S=.S.o)

all: kernel.o

kernel.o: $(OBJ)
	$(LD) -r -o kernel.o $^


%.c.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.asm.o: %.asm
	$(ASM) -f elf64 $< -o $@

%.S.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@