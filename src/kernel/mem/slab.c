#include "mem/mem.h"
#include "mem/pmm.h"
#include "mem/vmm.h"
#include "mem/slab.h"

static void slab_list_remove(slab_t** head, slab_t* node) {
    if (*head == node) *head = node->next;
    if (node->next) node->next->prev = node->prev;
    if (node->prev) node->prev->next = node->next;
    node->next = node->prev = NULL;
}

static void slab_list_insert(slab_t** head, slab_t* node) {
    node->next = *head;
    if (*head) (*head)->prev = node;
    *head = node;
    node->prev = NULL;
}

/**
 * @brief Sets up a named, dedicated cache for a specific structure size.
 */

void kmem_cache_init(kmem_cache_t* cache, size_t object_size) {
    // Objects must be large enough to hold a pointer to the next free object
    if (object_size < sizeof(void*)) {
        object_size = sizeof(void*);
    }
    cache->object_size = (object_size + 7) & ~7; // align to 8 bytes

    size_t usable_space = PAGE_SIZE - sizeof(slab_t);
    cache->objs_per_slab = usable_space / cache->object_size;

    cache->partial_slabs = NULL;
    cache->full_slabs = NULL;
    cache->empty_slabs = NULL;
}

static slab_t* kmem_cache_grow(kmem_cache_t* cache) {
    void* phys_frame = pmm_alloc_frame();
    if (!phys_frame) return NULL;

    // direct mapping strategy
    void* page_virt = P2V(phys_frame);

    // Embed the slab header at the start of the page
    slab_t* slab = (slab_t*)page_virt;
    slab->num_allocated = 0;
    slab->parent_cache = cache;
    slab->next = slab->prev = NULL;

    // start data slots immediately after the slab header
    uintptr_t first_object_addr = (uintptr_t)page_virt + sizeof(slab_t);
    first_object_addr = (first_object_addr + 7) & ~7; // align to 8 bytes

    slab->first_free_obj = (void*)first_object_addr;

    // intrusive free list generation:
    // loop through every object slot and write the address of the next slot into it
    uintptr_t current = first_object_addr;
    for (size_t i = 0; i < cache->objs_per_slab - 1; i++) {
        uintptr_t next = current + cache->object_size;
        *(void**)current = (void*)next; // write next pointer into current slot
        current = next;
    }
    *(void**)current = NULL; // last slot points to NULL

    // insert this fresh slab into our empty tracking list
    slab_list_insert(&cache->empty_slabs, slab);
    return slab;
}

void* kmem_cache_alloc(kmem_cache_t* cache) {
    slab_t* slab = NULL;

    // 1. try targeting partial slabs first
    if (cache->partial_slabs) {
        slab = cache->partial_slabs;
    }

    // 2. fall back to an empty slab if no partial entries exist
    else if (cache->empty_slabs) {
        slab = cache->empty_slabs;
        slab_list_remove(&cache->empty_slabs, slab);
        slab_list_insert(&cache->partial_slabs, slab);
    }

    // 3. no slabs ready. grow the cache dynamically
    else {
        slab = kmem_cache_grow(cache);
        if (!slab) return NULL;
        slab_list_remove(&cache->empty_slabs, slab);
        slab_list_insert(&cache->partial_slabs, slab);
    }

    void* obj = slab->first_free_obj;
    slab->first_free_obj = *(void**)obj; // update head of free list to next object
    slab->num_allocated++;    

    // reevaluate slab state: if slab is now full, move it to the full list
    if (slab->num_allocated == cache->objs_per_slab) {
        slab_list_remove(&cache->partial_slabs, slab);
        slab_list_insert(&cache->full_slabs, slab);
    }
    return obj;
}

void kmem_cache_free(kmem_cache_t* cache, void* obj) {
    if (!obj) return;

    // 1. Since page headers are embedded right at the start of the 4KiB boundary, we page align the object pointer down to get the slab header
    slab_t* slab = (slab_t*)((uintptr_t)obj & (PAGE_SIZE - 1));

    // link this object back to the front of the slab's free list
    *(void**)obj = slab->first_free_obj;
    slab->first_free_obj = obj;

    size_t old_allocated = slab->num_allocated;
    slab->num_allocated--;

    // readjust slab tracking states across lists
    if (old_allocated == cache->objs_per_slab) {
        // was completely full, now has a free slot. move from full to partial
        slab_list_remove(&cache->full_slabs, slab);
        slab_list_insert(&cache->partial_slabs, slab);
    } else if (slab->num_allocated == 0) {
        // slab is now completely free. move from partial to empty
        slab_list_remove(&cache->partial_slabs, slab);
        slab_list_insert(&cache->empty_slabs, slab);
 
        // completely free the page frame back to your PMM right here
        slab_list_remove(&cache->empty_slabs, slab);
        pmm_free_frame((void*)V2P(slab));
    }
}