#include <stddef.h>
#include <stdint.h>

#include <config.h>
#include <memory.h>
#include <kprintf.h>

#ifdef DEBUG
#include <debug.h>
#endif

#include "mmdef.h"

#define KHEAP_START 0xE1000000U
#define KHEAP_MAX 0xF0000000U

#define TYPE_NORMAL 0
#define TYPE_DMA (1 << 0)

#define div_up(a, b) (((a)-1) / (b) + 1)

typedef struct slab_t slab_t;
typedef struct cache_t cache_t;

struct slab_t
{
    slab_t *next;
    slab_t *prev;

    uint32_t free_count;

    void *free_obj;
    cache_t *cache;

    // 12 bytes reserved
};

struct cache_t
{
    cache_t *next;
    cache_t *prev;

    size_t obj_size;
    size_t slab_size;

    slab_t *full_slabs;
    slab_t *particial_slabs;
    slab_t *free_slabs;

    uint32_t type;
};

cache_t *caches;
void *heap_end = (void *)KHEAP_START;

static uint32_t slab_sizes[] = {
    PAGE_SIZE,     // 8
    PAGE_SIZE,     // 16
    PAGE_SIZE,     // 32
    PAGE_SIZE,     // 64
    PAGE_SIZE,     // 128
    PAGE_SIZE * 2, // 256
    PAGE_SIZE * 2, // 512
    PAGE_SIZE * 2, // 1024
    PAGE_SIZE * 4, // 2048
    PAGE_SIZE * 4, // 4096
    PAGE_SIZE * 8, // 8192
    PAGE_SIZE * 8, // 16384
};

static void *increase_heap(uint32_t size)
{
    if ((uint8_t *)heap_end + size > (uint8_t *)KHEAP_MAX)
    {
#ifdef DEBUG_MM
        dbg_kprintf("%s error, excided KHEAP_MAX bound\n", __func__);
#endif
        return NULL;
    }

    heap_end = (uint8_t *)heap_end + size;
    return (uint8_t *)heap_end - size;
}

static void slab_init(cache_t *cache, slab_t *slab, void *slab_adr)
{
    void **i = slab->free_obj;
    for (; (size_t)i < (size_t)slab_adr + cache->slab_size; i += (cache->obj_size / sizeof(size_t)))
    {
        *i = i + (cache->obj_size / sizeof(void *));
    }
    i -= (cache->obj_size / sizeof(void *));
    *i = NULL;
}

static cache_t *cache_create(size_t obj_size, uint32_t type)
{
#ifdef DEBUG_MM
    dbg_kprintf("%s call(obj_size: %#x, type: %#x)\n", __func__, obj_size, type);
#endif

    uint32_t size_order = 0;
    size_t size = obj_size;
    for (; size > 1; ++size_order)
    {
        size /= 2;
    }

    cache_t *cache = caches;
    cache_t *new_cache = kmalloc(sizeof(cache_t), 0);
    if (new_cache == NULL)
    {
#ifdef DEBUG_MM
        dbg_kprintf("%s error, kmalloc returned NULL on allocating header\n", __func__);
#endif
        return NULL;
    }

    while (obj_size > cache->obj_size)
    {
        if (cache->next != NULL)
        {
            cache = cache->next;
        }
        else
        {
            *new_cache = (cache_t){NULL, cache, obj_size,
                                   slab_sizes[size_order - 3],
                                   NULL, NULL, NULL, type};
            cache->next = new_cache;
            return new_cache;
        }
    }

    *new_cache = (cache_t){cache, cache->prev, obj_size,
                           slab_sizes[size_order - 3],
                           NULL, NULL, NULL, type};

    if (cache->prev != NULL)
    {
        cache->prev->next = new_cache;
    }

    cache->prev = new_cache;

    while (caches->prev != NULL)
    {
        caches = caches->prev;
    }

#ifdef DEBUG_MM
    dbg_kprintf("New cache: %p\n", new_cache);
#endif

    return new_cache;
}

static slab_t *cache_grow(cache_t *cache)
{
#ifdef DEBUG_MM
    dbg_kprintf("%s call(cache: %p)\n", __func__, cache);
#endif

    slab_t *new_slab;

    void *slab_region = increase_heap(cache->slab_size);
    if (slab_region == NULL)
    {
        return NULL;
    }

    uint32_t alloc_flags = 0;

    if ((cache->type & TYPE_DMA) != 0)
    {
        alloc_flags |= MM_DMA;
    }

    if (cache->slab_size == PAGE_SIZE)
    {
        void *phys_page = alloc_page(alloc_flags);
        if (phys_page == NULL)
        {
            return NULL;
        }
        map_page(phys_page, slab_region, PAGE_PRESENT | PAGE_WRITABLE);
        new_slab = (slab_t *)slab_region;

        void *first_free_obj = (uint8_t *)slab_region + cache->obj_size * div_up(sizeof(slab_t), cache->obj_size);
        uint32_t free_count = (cache->slab_size - ((size_t)first_free_obj & 0xFFF)) / cache->obj_size;
        *new_slab = (slab_t){NULL, NULL, free_count, first_free_obj, cache};
    }
    else
    {
        void *phys_mem = alloc_pages(cache->slab_size, alloc_flags);
        if (phys_mem == NULL)
        {
            return NULL;
        }

        void *virt_mem = slab_region;
        for (size_t i = cache->slab_size; i > 0; i -= PAGE_SIZE)
        {
            map_page(phys_mem, virt_mem, PAGE_PRESENT | PAGE_WRITABLE);
            phys_mem = (uint8_t *)phys_mem +  PAGE_SIZE;
            virt_mem = (uint8_t *)virt_mem + PAGE_SIZE;
        }

        new_slab = kmalloc(sizeof(slab_t), 0);
        if (new_slab == NULL)
        {
            return NULL;
        }
        uint32_t free_count = cache->slab_size / cache->obj_size;
        *new_slab = (slab_t){NULL, NULL, free_count, slab_region, cache};
    }

    slab_init(cache, new_slab, slab_region);

    cache->free_slabs = new_slab;

    void **heapinfo_pointer = get_heapinfo_pointer(slab_region);

    if (get_phys_adr(heapinfo_pointer) == NULL)
    {
        void *phys_page = alloc_page(MM_STANDART);
        if (phys_page == NULL)
        {
            return NULL;
        }
        map_page(phys_page, (void *)((size_t)heapinfo_pointer & (~0xFFFU)), PAGE_PRESENT | PAGE_WRITABLE);
    }

    for (uint32_t i = cache->slab_size; i > 0; i -= PAGE_SIZE)
    {
        *heapinfo_pointer = (void *)new_slab;
        ++heapinfo_pointer;
    }

#ifdef DEBUG_MM
    dbg_kprintf("New slab: %p\n", new_slab);
#endif

    return new_slab;
}

static inline void *slab_alloc(slab_t *slab)
{
    void *new_block = slab->free_obj;
    void *next_free = (void *)*(size_t *)slab->free_obj;

    slab->free_obj = next_free;
    --slab->free_count;

    return new_block;
}

static void *cache_alloc(cache_t *cache)
{
    slab_t *alloc_slab;

    if (cache->particial_slabs == NULL)
    {
        if (cache->free_slabs == NULL)
        {
            alloc_slab = cache_grow(cache);
            if (alloc_slab == NULL)
            {
                return NULL;
            }

            cache->particial_slabs = alloc_slab;
            cache->free_slabs = NULL;
        }
        else
        {
            alloc_slab = cache->free_slabs;

            if (alloc_slab->next != NULL)
            {
                cache->free_slabs = alloc_slab->next;
                alloc_slab->next->prev = NULL;
            }
            else
            {
                cache->free_slabs = NULL;
            }

            alloc_slab->next = NULL;
            alloc_slab->prev = NULL;

            cache->particial_slabs = alloc_slab;
        }
    }
    else
    {
        alloc_slab = cache->particial_slabs;
    }

    void *new_block = slab_alloc(alloc_slab);

    if (alloc_slab->free_count == 0)
    {
        cache->particial_slabs = alloc_slab->next;

        if (alloc_slab->next != NULL)
        {
            alloc_slab->next->prev = NULL;
        }

        if (cache->full_slabs != NULL)
        {
            cache->full_slabs->prev = alloc_slab;
        }
        alloc_slab->next = cache->full_slabs;
        cache->full_slabs = alloc_slab;
    }

    return new_block;
}

void *kmalloc(size_t size, uint32_t flags)
{
#ifdef DEBUG_MM
    dbg_kprintf("\n%s call(size: %u, flags: %#X)\n", __func__, size, flags);
#endif

    uint32_t cache_type = TYPE_NORMAL;
    size_t obj_size = 8;
    while (obj_size < size)
    {
        obj_size <<= 1;
    }

    if ((flags & MM_DMA) != 0)
    {
        cache_type |= TYPE_DMA;
    }

    cache_t *cache = caches;
    while ((obj_size != cache->obj_size) | (cache_type != cache->type))
    {
        if (cache->next == NULL)
        {
            cache = cache_create(obj_size, cache_type);
            if (cache == NULL)
            {
                return NULL;
            }
            break;
        }
        else
        {
            cache = cache->next;
        }
    }

    if (cache == NULL)
    {
        return NULL;
    }

    void *new_block = cache_alloc(cache);
    if (new_block == NULL)
    {
        return NULL;
    }

    #ifdef DEBUG_MM
    dbg_kprintf("Allocated: %p\n", new_block);
    #endif

    return new_block;
}

static inline void slab_free(slab_t *slab, void *obj)
{
    if ((uint32_t)obj % slab->cache->obj_size != 0)
    {
#ifdef DEBUG_MM
        dbg_kprintf("%s error, pointer is not properly aligned\n", __func__);
#endif
        return;
    }

    *(size_t *)obj = (size_t)slab->free_obj;

    slab->free_obj = obj;

    ++slab->free_count;
}

void kfree(void *obj)
{
#ifdef DEBUG_MM
    dbg_kprintf("\n%s call(obj: %p)\n", __func__, obj);
#endif

    size_t *heap_info_pointer = (size_t *)(HEAP_INFO_REGION + (((size_t)obj & (~0xFFFU)) - KERNEL_REGION) / 1024);

    slab_t *slab = (slab_t *)*heap_info_pointer;

    slab_free(slab, obj);

    cache_t *cache = slab->cache;

    uint32_t free_count = 0;
    if (cache->slab_size == PAGE_SIZE)
    {
        free_count = (cache->slab_size - ((size_t)slab->free_obj & 0xFFF)) / cache->obj_size;
    }
    else
    {
        free_count = cache->slab_size / cache->obj_size;
    }

    if (slab->free_count == free_count)
    {
        if (slab->prev != NULL)
        {
            slab->prev->next = slab->next;
        }
        else
        {
            cache->particial_slabs = slab->next;
        }

        if (slab->next != NULL)
        {
            slab->next->prev = slab->prev;
        }

        if (cache->free_slabs != NULL)
        {
            cache->free_slabs->prev = slab;
            slab->next = cache->free_slabs;
            cache->free_slabs = slab;
            slab->prev = NULL;
        }
        else
        {
            cache->free_slabs = slab;
            slab->next = NULL;
            slab->prev = NULL;
        }
    }
}

void init_kheap()
{
    slab_t *slab_32 = increase_heap(PAGE_SIZE);

    map_page(alloc_page(MM_STANDART), slab_32, PAGE_PRESENT | PAGE_WRITABLE);

    cache_t *cache_32 = (cache_t *)((uint8_t *)slab_32 + 32);

    *cache_32 = (cache_t){NULL, NULL, 32, PAGE_SIZE, NULL, slab_32, NULL, TYPE_NORMAL};
    *slab_32 = (slab_t){NULL, NULL, (cache_32->slab_size - cache_32->obj_size) / cache_32->obj_size, (uint8_t *)cache_32 + 32, cache_32};

    slab_init(cache_32, slab_32, slab_32);

    caches = cache_32;

    size_t kernel_code_region = 0x1000000 / 0x1000 * sizeof(size_t);
    map_page(alloc_page(MM_STANDART), (void *)(HEAP_INFO_REGION + kernel_code_region), PAGE_PRESENT | PAGE_WRITABLE);
    *(size_t *)(HEAP_INFO_REGION + kernel_code_region) = (size_t)slab_32;
}
