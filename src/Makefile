.PHONY: kernel/kernel.o libk/libk.a



kernel.elf: libk/libk.a kernel/kernel.o
	ld -o kernel.elf -T linker.ld kernel/kernel.o libk/libk.a -Llibk -lk 

kernel/kernel.o: linker.ld
	$(MAKE) -C kernel

libk/libk.a:
	$(MAKE) -C libk