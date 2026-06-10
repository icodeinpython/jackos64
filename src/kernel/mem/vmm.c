#include <stdint.h>
#include "mem/mem.h"
#include "mem/pmm.h"
#include "mem/vmm.h"
#include <string.h>

static inline uint64_t pml4_index(uintptr_t virt) { return (virt >> 39) & 0x1FF; }
static inline uint64_t pdpt_index(uintptr_t virt) { return (virt >> 30) & 0x1FF; }
static inline uint64_t pd_index(uintptr_t virt) { return (virt >> 21) & 0x1FF; }
static inline uint64_t pt_index(uintptr_t virt) { return (virt >> 12) & 0x1FF; }

page_table_t* kernel_pml4 = NULL;

static inline void tlb_flush_single(uintptr_t addr) {
    __asm__ volatile ("invlpg (%0)" :: "r" (addr) : "memory");
}

/**
 * @brief Inner engine: Traverses down page table levels. 
 * Returns level 1 page entry pointer, or Level 2 entry pointer if huge page.
 */
static pt_entry_t* vmm_walk(page_table_t* pml4, uintptr_t virt, int allocate_missing, int huge_page) {
    // 1. pml4 -> pdpt
    uint64_t pml4_idx = pml4_index(virt);
    if (!(pml4->entries[pml4_idx] & PTE_PRESENT)) {
        if (!allocate_missing) return NULL;
        void* new_frame = pmm_alloc_frame();
        if (!new_frame) return NULL;
        memset(P2V(new_frame), 0, PAGE_SIZE);
        pml4->entries[pml4_idx] = (uintptr_t)new_frame | PTE_PRESENT | PTE_WRITABLE | PTE_USER;
    }
    page_table_t* pdpt = (page_table_t*)P2V(pml4->entries[pml4_idx] & PTE_FRAME);

    // 2. pdpt -> pdT
    uint64_t pdpt_idx = pdpt_index(virt);
    if (!(pdpt->entries[pdpt_idx] & PTE_PRESENT)) {
        if (!allocate_missing) return NULL;
        void* new_frame = pmm_alloc_frame();
        if (!new_frame) return NULL;
        memset(P2V(new_frame), 0, PAGE_SIZE);
        pdpt->entries[pdpt_idx] = (uintptr_t)new_frame | PTE_PRESENT | PTE_WRITABLE | PTE_USER;
    }
    page_table_t* pdt = (page_table_t*)P2V(pdpt->entries[pdpt_idx] & PTE_FRAME);


    // 3. pdT -> pt or huge page
    uint64_t pd_idx = pd_index(virt);

    if (huge_page) {
        return &pdt->entries[pd_idx];
    }

    if (!(pdt->entries[pd_idx] & PTE_PRESENT)) {
        if (!allocate_missing) return NULL;
        void* new_frame = pmm_alloc_frame();
        if (!new_frame) return NULL;
        memset(P2V(new_frame), 0, PAGE_SIZE);
        pdt->entries[pd_idx] = (uintptr_t)new_frame | PTE_PRESENT | PTE_WRITABLE | PTE_USER;
    } else if (pdt->entries[pd_idx] & PTE_HUGE_PAGE) {
        // already mapped as a huge page, cannot map as normal page
        return NULL;
    }
    page_table_t* pt = (page_table_t*)P2V(pdt->entries[pd_idx] & PTE_FRAME);

    return &pt->entries[pt_index(virt)];
}

int vmm_map(page_table_t* pml4, uintptr_t virt, uintptr_t phys, uint64_t flags) {
    if ((virt % PAGE_SIZE) != 0 || (phys % PAGE_SIZE) != 0) {
        return 0; // addresses must be page-aligned
    }

    pt_entry_t* pte = vmm_walk(pml4, virt, 1, 0);
    if (!pte) return 0; // failed to allocate page tables

    *pte = (phys & PTE_FRAME) | flags | PTE_PRESENT;
    tlb_flush_single(virt);
    return 1;
}

int vmm_map_huge(page_table_t* pml4, uintptr_t virt, uintptr_t phys, uint64_t flags) {
    if ((virt % 0x200000) != 0 || (phys % 0x200000) != 0) {
        return 0; // addresses must be 2MB-aligned
    }

    pt_entry_t* pde = vmm_walk(pml4, virt, 1, 1);
    if (!pde) return 0; // failed to allocate page tables

    *pde = (phys & PTE_FRAME) | flags | PTE_PRESENT | PTE_HUGE_PAGE;
    tlb_flush_single(virt);
    return 1;
}

int vmm_unmap(page_table_t* pml4, uintptr_t virt) {
    // check for standard page mapping walk
    pt_entry_t* entry = vmm_walk(pml4, virt, 0, 0);
    if (entry && (*entry & PTE_PRESENT)) {
        *entry = 0;
        tlb_flush_single(virt);
        return 1;
    }

    // check for huge page mapping walk
    entry = vmm_walk(pml4, virt, 0, 1);
    if (entry && (*entry & PTE_PRESENT)) {
        *entry = 0;
        tlb_flush_single(virt);
        return 1;
    }

    return 0; // page wasn't mapped
}

void vmm_init(e820_entry_t* mem_map, uint32_t entry_count, uintptr_t kernel_phys_start, uintptr_t kernel_size) {
    // 1. Allocate a fresh frame to serve as our permanent standalone Root PML4 mapping table
    void* new_pml4_frame = pmm_alloc_frame();
    kernel_pml4 = (page_table_t*)P2V(new_pml4_frame);
    memset(kernel_pml4, 0, PAGE_SIZE);

    // 2. Locate total system RAM capacity boundaries from data array
    uint64_t highest_address = 0;
    for (uint32_t i = 0; i < entry_count; i++) {
        if (mem_map[i].type == 1) { // Usable RAM type
            uint64_t limit = mem_map[i].base_addr + mem_map[i].length;
            if (limit > highest_address) highest_address = limit;
        }
    }

    // 3. Construct permanent Direct-Mapping Window across total available systems memory space 
    // Uses efficient 2MiB huge page chunks to mimic the early boot architecture layout
    for (uintptr_t phys = 0; phys < highest_address; phys += 0x200000) {
        vmm_map_huge(kernel_pml4, phys + DIRECT_MAP_OFFSET, phys, PTE_WRITABLE);
    }

    // 4. Map Higher-Half Executable Kernel binary memory space (0xFFFFFFFF80000000)
    // Uses granular 4KiB alignments to keep kernel boundary configurations accurate
    uintptr_t rounded_kernel_size = (kernel_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    for (uintptr_t offset = 0; offset < rounded_kernel_size; offset += PAGE_SIZE) {
        vmm_map(kernel_pml4, 0xFFFFFFFF80000000 + offset, kernel_phys_start + offset, PTE_WRITABLE);
    }

    // 5. CR3 Context Swap Execution: Drops early boot assembly scaffolding structures permanently
    uintptr_t new_pml4_phys = (uintptr_t)new_pml4_frame;
    __asm__ volatile(
        "mov %0, %%cr3" 
        : 
        : "r"(new_pml4_phys) 
        : "memory"
    );
}