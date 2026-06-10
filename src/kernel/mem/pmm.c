#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "mem/mem.h"
#include "mem/pmm.h"
#include "bootloader.h"
#include "kernel.h"

uint8_t* pmm_bitmap = NULL;
uint64_t pmm_max_blocks = 0;


void pmm_init(const struct mmap* mmap) {
    uint64_t highest_address = 0;

    e820_entry_t *entries = (e820_entry_t*)mmap->mmap;

    printf_serial("PMM: Initializing with %p mmap entries\n", mmap->mmap);

    // dump mmap
    for (uint32_t i = 0; i < mmap->mmap_count; i++) {
        printf_serial("mmap[%d]: base=%#x, length=%#x, type=%#x\n", i, entries[i].base_addr, entries[i].length, entries[i].type);
    }

    // 1. Find the top of memory
    for (uint32_t i = 0; i < mmap->mmap_count; i++) {
        if (entries[i].type == E820_TYPE_USABLE) {
            uint64_t top = entries[i].base_addr + entries[i].length;
            if (top > highest_address) {
                highest_address = top;
            }
        }
    }

    pmm_max_blocks = highest_address / 4096;
    uint64_t bitmap_size = pmm_max_blocks / 8;

    // 2. Anchor the bitmap right after the kernel executable
    pmm_bitmap = (uint8_t*)_kernel_end_virt;

    printf_serial("PMM: highest_address=%#x, max_blocks=%#x, bitmap_size=%#x\n", highest_address, pmm_max_blocks, bitmap_size);

    // Calculate where actual free allocations can safely start
    uintptr_t free_memory_start = ((uintptr_t)pmm_bitmap - VIRT_BASE) + bitmap_size;
    free_memory_start = (free_memory_start + 4095) & ~4095; // Page align

    /* ---------------------------------------------------------------------
     * OPTIMIZATION 1: Default the entire map to reserved (0xFF)
     * --------------------------------------------------------------------- */
    memset(pmm_bitmap, 0xFF, bitmap_size);

    // 4. Punch holes for the free memory blocks
    for (uint32_t i = 0; i < mmap->mmap_count; i++) {
        if (entries[i].type == E820_TYPE_USABLE) {
            printf_serial("Usable block: base=%#x, length=%#x\n", entries[i].base_addr, entries[i].length);
            uint64_t start_block = entries[i].base_addr / 4096;
            uint64_t block_count = entries[i].length / 4096;

            /* ---------------------------------------------------------------------
             * OPTIMIZATION 2: Clear byte-aligned sections via memset
             * --------------------------------------------------------------------- */
            uint64_t current_block = start_block;
            uint64_t remaining_blocks = block_count;

            // Handle unaligned head blocks individually until we hit a byte boundary
            while (remaining_blocks > 0 && (current_block % 8) != 0) {
                pmm_bitmap[current_block / 8] &= ~(1 << (current_block % 8));
                current_block++;
                remaining_blocks--;
            }

            // High-speed blast across full bytes using our vector-optimized memset
            uint64_t full_bytes_to_clear = remaining_blocks / 8;
            if (full_bytes_to_clear > 0) {
                memset(&pmm_bitmap[current_block / 8], 0x00, full_bytes_to_clear);
                current_block += full_bytes_to_clear * 8;
                remaining_blocks -= full_bytes_to_clear * 8;
            }

            // Handle unaligned tail blocks individually
            while (remaining_blocks > 0) {
                pmm_bitmap[current_block / 8] &= ~(1 << (current_block % 8));
                current_block++;
                remaining_blocks--;
            }
        }
    }

    /* ---------------------------------------------------------------------
     * OPTIMIZATION 3: Protect the Kernel, early boot pages, and the bitmap
     * --------------------------------------------------------------------- */
    uint64_t reserved_pages = free_memory_start / 4096;
    uint64_t full_bytes_to_reserve = reserved_pages / 8;
    printf_serial("PMM initialization complete. Free memory starts at %#x\n", free_memory_start);

    // Bulk set the entire lower region using memset
    if (full_bytes_to_reserve > 0) {
        memset(pmm_bitmap, 0xFF, full_bytes_to_reserve);
    }

    // Pick up any leftover pages that don't perfectly fill a whole byte
    for (uint64_t b = full_bytes_to_reserve * 8; b < reserved_pages; b++) {
        pmm_bitmap[b / 8] |= (1 << (b % 8));

    }

    // dump_pmm_bitmap();

    // reserve pages for framebuffer which will be identity mapped
    if ((uintptr_t)bootinfo.video.framebuffer > highest_address) {
        printf_serial("Warning: Framebuffer address %#x is above the highest address %#x detected in the memory map. Skipping reservation.\n", bootinfo.video.framebuffer, highest_address);
    } else {
        uint64_t fb_start_block = (uintptr_t)bootinfo.video.framebuffer / 4096;
        uint64_t fb_blocks = (bootinfo.video.screen_size + 4095) / 4096; // Round up to nearest page
        printf_serial("Reserving framebuffer blocks: start=%#x, blocks=%#x\n", fb_start_block, fb_blocks);
        for (uint64_t b = fb_start_block; b < fb_start_block + fb_blocks; b++) {
            pmm_bitmap[b / 8] |= (1 << (b % 8));
        }
    }
}


// prints over serial ranges of free and used blocks in the bitmap for debugging
void dump_pmm_bitmap() {
    printf_serial("PMM Bitmap Dump:\n");
    for (uint64_t i = 0; i < pmm_max_blocks; i++) {
        uint8_t byte = pmm_bitmap[i / 8];
        uint8_t bit = (byte >> (i % 8)) & 1;
        printf_serial("%c", bit ? 'X' : '.');
        if ((i + 1) % 64 == 0) {
            printf_serial("\n");
        }
    }
    printf_serial("\n");
}

// returns a pointer to a physical address of a page frame
void* pmm_alloc_frame() {
    uint64_t* bitmap64 = (uint64_t*)pmm_bitmap;
    uint64_t max_words = pmm_max_blocks / 64;

    for (uint64_t i = 0; i < max_words; i++) {
        // if this word is fully allocated, skip it
        if (bitmap64[i] == 0xFFFFFFFFFFFFFFFF) {
            continue;
        }
        uint64_t free_bit = __builtin_ctzll(~bitmap64[i]); // Count trailing zeros to find first free bit

        uint64_t block_index = (i * 64) + free_bit;
        if (block_index >= pmm_max_blocks) {
            return NULL; // Out of memory
        }

        pmm_bitmap[block_index / 8] |= (1 << (block_index % 8)); // Mark block as allocated
        return (void*)(block_index * 4096);
    }
    return NULL; // Out of memory
}


/**
 * @brief Allocates multiple contiguous physical page frames.
 * @param count The number of consecutive 4KiB frames needed (e.g., for DMA or page tables).
 * @return The physical starting address of the block, or NULL if a large enough block wasn't found.
 */
void* pmm_alloc_frames(size_t count) {
    if (count == 0) return NULL;
    if (count == 1) return pmm_alloc_frame();

    uint64_t consecutive_free = 0;
    uint64_t start_block = 0;
    for (uint64_t i = 0; i < pmm_max_blocks; i++) {
        if ((pmm_bitmap[i/8] & (1 << (i % 8))) == 0) {
            // this block is free
            if (consecutive_free == 0) {
                start_block = i;
            }
            consecutive_free++;
            if (consecutive_free == count) {
                // found a chunk
                for (uint64_t b = start_block; b < start_block + count; b++) {
                    pmm_bitmap[b/8] |= (1 << (b % 8));
                }
                return (void*)(start_block * 4096);
            }
        } else {
            consecutive_free = 0; // reset count if we hit an allocated block
        }
    }
    return NULL;
}

void pmm_free_frame(void* phys_addr) {
    uintptr_t addr = (uintptr_t)phys_addr;
    if (addr % PAGE_SIZE != 0) {
        // kernel panic
        hcf();
    }
    uint64_t block_index = addr / PAGE_SIZE;
    if (block_index < pmm_max_blocks) {
        pmm_bitmap[block_index / 8] &= ~(1 << (block_index % 8)); // Mark block as free
    }
}

void pmm_free_frames(void* phys_addr, size_t count) {
    uintptr_t addr = (uintptr_t)phys_addr;
    if (addr % PAGE_SIZE != 0) return;

    uint64_t start_block = addr / PAGE_SIZE;

    for (size_t i = 0; i < count; i++) {
        uint64_t block_index = start_block + i;
        if (block_index < pmm_max_blocks) {
            pmm_bitmap[block_index / 8] &= ~(1 << (block_index % 8));
        }
    }
}