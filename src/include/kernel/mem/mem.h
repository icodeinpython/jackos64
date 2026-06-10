#pragma once

#include <stdint.h>

#define PAGE_SIZE 4096
#define VIRT_BASE 0xFFFFFFFF80000000

extern uint8_t _kernel_end_virt[];
extern uint8_t _kernel_end_phys[];
extern uint8_t _kernel_start_virt[];
extern uint8_t _kernel_start_phys[];

void mem_init();