#include "mem/pmm.h"
#include "mem/mem.h"
#include "mem/vmm.h"
#include "mem/slab.h"
#include "kernel.h"
#include <bootloader.h>

void mem_init() {
    pmm_init(&bootinfo.mmap);
    vmm_init(bootinfo.mmap.mmap, bootinfo.mmap.mmap_count, (uintptr_t)_kernel_start_phys, (uintptr_t)_kernel_end_phys - (uintptr_t)_kernel_start_phys);
    kmalloc_init();
}