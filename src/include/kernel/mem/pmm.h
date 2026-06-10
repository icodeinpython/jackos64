#pragma once

#include "bootloader.h"
#include "mem/mem.h"

#include <stdint.h>
#include <stddef.h>

void pmm_init(const struct mmap* memory_map);
void dump_pmm_bitmap();
void* pmm_alloc_frame();
void* pmm_alloc_frames(size_t count);
void pmm_free_frame(void* phys_addr);
void pmm_free_frames(void* phys_addr, size_t count);

#define E820_TYPE_USABLE       1
#define E820_TYPE_RESERVED     2
#define E820_TYPE_ACPI_RECLAIM 3
#define E820_TYPE_ACPI_NVS     4
#define E820_TYPE_BAD          5

extern uint8_t* pmm_bitmap;
extern uint64_t pmm_max_blocks;