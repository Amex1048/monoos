#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <memory.h>
#include <string.h>
#include <boot.h>

#include "mmdef.h"

#define PMM_BITMAP_SIZE (PHYS_MEM_LIMIT / (PAGE_SIZE * 8))
#define div_up(a, b) (((a)-1) / (b) + 1)

uint8_t pmm_bitmap[PMM_BITMAP_SIZE];
uint32_t cached_pos = 0;

static inline void reserve_page(uint32_t index)
{
    uint32_t pos = index >> 3;
    pmm_bitmap[pos] += (uint8_t)(1 << (index % 8));
}

static inline void unreserve_page(uint32_t index)
{
    uint32_t pos = index >> 3;
    pmm_bitmap[pos] -= (uint8_t)(1 << (index % 8));
}

static inline uint8_t page_reserved(uint32_t index)
{
    uint32_t pos = index >> 3;
    return pmm_bitmap[pos] & (uint8_t)(1 << (index % 8));
}

static void set_bitmap_hole(uint32_t offset, uint32_t size)
{
    for (; size > 0; --size)
    {
        reserve_page(offset);
        ++offset;
    }
}

void *alloc_pages(size_t size, uint32_t flags)
{
    if (available_mem < size / 1024)
    {
        return NULL;
    }

    if (((flags & MM_DMA) != 0) & (size > 64 * 1024))
    {
        return NULL;
    }

    size_t count = div_up(size, PAGE_SIZE);

    uint32_t pos = 0;
    uint32_t index = 0;
    uint32_t dma_pos = 0;
    uint32_t start_index = 0;

    while (true)
    {
        pos = index >> 3;
        while (pmm_bitmap[pos] == 0xFF)
        {
            ++pos;
            index = pos << 3;
        }

        dma_pos = pos & (~(uint32_t)1);

        while (page_reserved(index))
        {
            ++index;
        }
        start_index = index;

        while (true)
        {
            if (page_reserved(index) == 0)
            {
                ++index;
            }
            else
            {
                break;
            }

            if (index > PHYS_MEM_LIMIT / PAGE_SIZE)
            {
                return NULL;
            }

            if (index - start_index >= count)
            {
                if ((flags & MM_DMA) != 0)
                {
                    uint32_t new_pos = (index - 1) >> 3;
                    if (new_pos - dma_pos <= 1)
                    {
                        goto done;
                    }
                    else
                    {
                        index = (dma_pos + 2) << 3;
                        break;
                    }
                }
                else
                {
                    goto done;
                }
            }
        }
    }

done:
    for (--index; index >= start_index; --index)
    {
        reserve_page(index);
    }
    available_mem -= count * (PAGE_SIZE / 1024);

    return (void *)(start_index * PAGE_SIZE);
}

void *alloc_page(uint32_t flags)
{
    if (available_mem < (PAGE_SIZE / 1024))
    {
        return NULL;
    }

    while (pmm_bitmap[cached_pos] == 0xFF)
    {
        ++cached_pos;
        if (cached_pos > (PHYS_MEM_LIMIT / PAGE_SIZE / sizeof(uint8_t)))
        {
            cached_pos = 0;
        }
    }
    uint32_t index = cached_pos << 3;

    while (page_reserved(index))
    {
        ++index;
    }

    reserve_page(index);
    available_mem -= (PAGE_SIZE / 1024);

    return (void *)(index * PAGE_SIZE);
}

void free_page(void *page)
{
    uint32_t index = (size_t)page / PAGE_SIZE;
    unreserve_page(index);
    available_mem += (PAGE_SIZE / 1024);

    if (index >> 3 < cached_pos)
    {
        cached_pos = index >> 3;
    }
}

void free_pages(void *pages, uint32_t count)
{
    for (; count > 0; --count)
    {
        free_page(pages);
        pages = (uint8_t *)pages + PAGE_SIZE;
    }
}

void init_pmm(mem_map *map)
{
    memset(pmm_bitmap, 0, PMM_BITMAP_SIZE);

    uint16_t low_reserved_pages = (uint16_t)div_up(map->low_mem_reserved, PAGE_SIZE / 1024U);
    uint16_t high_reserved_pages = (uint16_t)div_up(map->high_mem_reserved, PAGE_SIZE / 1024U);
    uint16_t low_pages = map->low_mem / (PAGE_SIZE / 1024);
    uint16_t high_pages = map->high_mem / (PAGE_SIZE / 1024);

    available_mem = (uint32_t)(map->low_mem - map->low_mem_reserved + map->high_mem - map->high_mem_reserved);

    set_bitmap_hole(0, low_reserved_pages);
    set_bitmap_hole(low_pages, (0x100000 / PAGE_SIZE) - low_pages);
    set_bitmap_hole(0x100000 / PAGE_SIZE, high_reserved_pages);
    set_bitmap_hole((0x100000 / PAGE_SIZE) + high_pages,
                    (PHYS_MEM_LIMIT / PAGE_SIZE) - ((0x100000 / PAGE_SIZE) + high_pages));
}
