.PHONY: kernel/kernel.o libk/libk.a



kernel.elf: libk/libk.a kernel/kernel.o
	ld -o kernel.elf -T linker.ld kernel/kernel.o --start-group libk/libk.a --end-group
	nm kernel.elf > kernel.sym

kernel/kernel.o: linker.ld
	$(MAKE) -C kernel

libk/libk.a:
	$(MAKE) -C libk

clean:
	$(MAKE) -C kernel clean
	$(MAKE) -C libk clean
	rm -f kernel.elf