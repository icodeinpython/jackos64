.PHONY: disk.img kernel

disk.img: kernel
	cp src/kernel.elf bootloader/kernel.elf
	$(MAKE) -C bootloader
	mv bootloader/disk.img disk.img

kernel:
	$(MAKE) -C src

qemu: disk.img
	qemu-system-x86_64 -hda disk.img

bochs: disk.img
	bochs -f bochsrc.txt -dbg_gui