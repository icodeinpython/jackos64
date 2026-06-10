#include <stdint.h>

typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed)) gdt_t;


typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) gdtr_t;


extern gdt_t gdt[3]; // Null, Code, Data (kernel only for now)
extern gdtr_t gdtr;

void gdt_init();
void gdt_set_descriptor(int index, uint8_t access, uint8_t gran);

extern void load_gdt();