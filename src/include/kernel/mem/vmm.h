#pragma once

#include <stdint.h>
#include "mem/mem.h"
#include "mem/pmm.h"

#define DIRECT_MAP_OFFSET VIRT_BASE

// converts a physical frame address to its cooresponding virtual address in kernel space
#define P2V(phys) ((void*)((uintptr_t)(phys) + DIRECT_MAP_OFFSET))

// converts a direct-mapped virtual address in kernel space to its cooresponding physical frame address
#define V2P(virt) ((uintptr_t)(virt) - DIRECT_MAP_OFFSET)

#define ENTRIES_PER_TABLE 512

typedef uint64_t pt_entry_t;

typedef struct {
    pt_entry_t entries[ENTRIES_PER_TABLE];
} page_table_t;

extern page_table_t* kernel_pml4;

#define PTE_PRESENT 0x1
#define PTE_WRITABLE 0x2
#define PTE_USER 0x4
#define PTE_HUGE_PAGE (1ULL << 7)
#define PTE_FRAME 0x000FFFFFFFFFF000ULL

void vmm_init(e820_entry_t* mmap, uint32_t mmap_count, uintptr_t kernel_phys_start, uint64_t kernel_size);
int vmm_map(page_table_t* pml4, uintptr_t virt, uintptr_t phys, uint64_t flags);
int vmm_map_huge(page_table_t* pml4, uintptr_t virt, uintptr_t phys, uint64_t flags);
int vmm_unmap(page_table_t* pml4, uintptr_t virt);