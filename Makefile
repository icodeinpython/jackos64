.PHONY: disk.img kernel clean qemu bochs

disk.img: kernel
	cp src/kernel.elf bootloader/kernel.elf
	$(MAKE) -C bootloader
	mv bootloader/disk.img disk.img

kernel:
	$(MAKE) -C src

qemu: disk.img
	qemu-system-x86_64 -drive file=disk.img,format=raw -serial stdio -enable-kvm -cpu host

qemu_dbg: disk.img
	qemu-system-x86_64 -drive file=disk.img,format=raw -s -S -monitor stdio -enable-kvm -cpu host

qemu_int: disk.img
	qemu-system-x86_64 -drive file=disk.img,format=raw -cpu Haswell,+avx,+avx2 -d int,cpu_reset -D qemu.log -no-reboot -no-shutdown

bochs: disk.img
	bochs -f bochsrc.txt -dbg_gui

clean:
	$(MAKE) -C src clean
	$(MAKE) -C bootloader clean
	rm -f disk.img