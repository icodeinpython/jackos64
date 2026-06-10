#include "mem/vmm.h"
#include "kernel.h"
#include "bootloader.h"
#include "drivers/fb.h"


/**
 * @brief Programs the AMD64 Page Attribute Table (PAT) safely.
 * * The IA32_PAT MSR (0x277) layout across split 32-bit registers:
 * - eax (low 32-bits):  Slot 3 | Slot 2 | Slot 1 | Slot 0
 * - edx (high 32-bits): Slot 7 | Slot 6 | Slot 5 | Slot 4
 * * We must explicitly capture and write back both halves to prevent clobbering
 * upper slots 4-7 on 64-bit hardware.
 */
static void init_pat_write_combining() {
    uint32_t eax, edx;
    uint32_t msr = 0x277; // IA32_PAT MSR
    
    __asm__ volatile (
        "rdmsr"
        : "=a"(eax), "=d"(edx)
        : "c"(msr)
    );

    // clear slot 2 (bits 18:16 in eax)
    eax &= ~(0x7ULL << 16);

    // inject memory type 0x01 (Write-Combining) into slot 2
    eax |= (0x1ULL << 16);

    __asm__ volatile (
        "wrmsr"
        :
        : "a"(eax), "d"(edx), "c"(msr)
        : "memory"
    );
}

static void video_map_framebuffer() {
    struct videoInfo* info = &bootinfo.video;

    if (!info || !info->framebuffer) return;

    // 1. PAT initialization
    init_pat_write_combining();

    uintptr_t fb_phys = (uintptr_t)info->framebuffer;
    uintptr_t fb_phys_aligned = fb_phys & ~(PAGE_SIZE - 1); // align down to 4KiB page boundary
    uintptr_t misalignment_offset = fb_phys - fb_phys_aligned;

    uintptr_t total_size = info->screen_size + misalignment_offset;
    uintptr_t page_count = (total_size + PAGE_SIZE - 1) / PAGE_SIZE;

    // define page attributes: 
    uint64_t wc_flags = PTE_PRESENT | PTE_WRITABLE  | PTE_CACHE_DISABLE; // Write-Combining with cache disabled to prevent stale reads


    for (uintptr_t i = 0; i < page_count; i++) {
        uintptr_t v_addr = VIRT_FRAMEBUFFER_BASE + (i * PAGE_SIZE);
        uintptr_t p_addr = fb_phys_aligned + (i * PAGE_SIZE);

        vmm_map(kernel_pml4, v_addr, p_addr, wc_flags);
    }

    info->framebuffer = (uint32_t*)(VIRT_FRAMEBUFFER_BASE + misalignment_offset);
}

void init_fb() {
    video_map_framebuffer();
}