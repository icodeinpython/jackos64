#pragma once

#include <stdint.h>
#include <stddef.h>

#include "mem/mem.h"

typedef struct slab {
    void* first_free_obj;
    size_t num_allocated;
    struct kmem_cache* parent_cache;
    struct slab* next;
    struct slab* prev;
} slab_t;

typedef struct kmem_cache {
    size_t object_size;
    size_t objs_per_slab;

    slab_t* partial_slabs;
    slab_t* full_slabs;
    slab_t* empty_slabs;
} kmem_cache_t;

void kmem_cache_init(kmem_cache_t* cache, size_t object_size);
void* kmem_cache_alloc(kmem_cache_t* cache);
void kmem_cache_free(kmem_cache_t* cache, void* obj);
void kmalloc_init();