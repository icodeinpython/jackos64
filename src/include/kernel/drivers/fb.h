#include "mem/vmm.h"

#define PTE_CACHE_DISABLE (1ULL << 4)

#define VIRT_FRAMEBUFFER_BASE 0xFFFFFFFFC0000000ULL

void init_fb();