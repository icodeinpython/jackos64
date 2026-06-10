#include <stdlib.h>
#include "mem/slab.h"
#include "mem/pmm.h"
#include "mem/vmm.h"

#define NUM_GENERIC_CACHES 8

kmem_cache_t generic_caches[NUM_GENERIC_CACHES];
size_t cache_sizes[NUM_GENERIC_CACHES] = {16, 32, 64, 128, 256, 512, 1024, 2048};

void kmalloc_init() {
    for (int i = 0; i < NUM_GENERIC_CACHES; i++) {
        kmem_cache_init(&generic_caches[i], cache_sizes[i]);
    }
}

void* kmalloc(size_t size) {
    // SLAB PATH: small allocations
    // route the size allocation to the closest matching bucket
    for (int i = 0; i < NUM_GENERIC_CACHES; i++) {
        if (size <= cache_sizes[i]) {
            return kmem_cache_alloc(&generic_caches[i]);
        }
    }


    // LARGE ALLOC PATH
    size_t total_bytes_needed = size + sizeof(size_t);
    size_t page_count = (total_bytes_needed + PAGE_SIZE - 1) / PAGE_SIZE;

    // allocate contiguous pages from the PMM
    void* phys_addr = pmm_alloc_frames(page_count);
    if (!phys_addr) return NULL; // out of memory

    // translate the physical address to a virtual address in the direct map region
    void* virt_addr = P2V(phys_addr);

    // write the page count into the first 8 bytes of the allocation for future reference during free
    *(size_t*)virt_addr = page_count;

    // return the pointer shifted forward by 8 bytes to hide the header from the caller
    return (void*)((uintptr_t)virt_addr + sizeof(size_t));
}

void kfree(void* ptr) {
    if (!ptr) return;

    uintptr_t addr = (uintptr_t)ptr;

    // Step 1: Detect if this is a large page allocation or a slab allocation.
    // If it's a large allocation, its pointer is offset from a 4KiB page boundary 
    // by exactly the size of our metadata header (sizeof(size_t) = 8 bytes).
    if ((addr % PAGE_SIZE) == sizeof(size_t)) {
        // This is a large allocation. We need to read the page count from the header and free the frames.
        void* raw_page_start = (void*)(addr - sizeof(size_t));
        size_t page_count = *(size_t*)raw_page_start;


        // Calculate the physical address of the start of this allocation
        uintptr_t phys_addr = V2P(raw_page_start);

        // Free the contiguous block of frames back to the PMM
        pmm_free_frames((void*)phys_addr, page_count);
        return;
    }

    // SLAB
    slab_t* slab = (slab_t*)(addr & ~(PAGE_SIZE - 1)); // find the slab header by aligning down to page boundary

    // follow the back pointer
    kmem_cache_t* cache = slab->parent_cache;
    kmem_cache_free(cache, ptr);
}