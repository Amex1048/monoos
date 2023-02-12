#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <memory.h>

#include "mmdef.h"

#define VHEAP_START 0xF0000000
#define VHEAP_MAX 0xFFBF0000

#define div_up(a, b) (((a)-1) / (b) + 1)

typedef struct vheader_t vheader_t;
struct vheader_t
{
    vheader_t *next;
    vheader_t *prev;

    size_t size;
    void *region;
};

vheader_t *headers;

static void insert_ordered_list(vheader_t *item)
{
    if (headers == NULL)
    {
        headers = item;
        item->prev = NULL;
        item->next = NULL;
        return;
    }

    vheader_t *header = headers;

    while (item->size > header->size)
    {
        if (header->next == NULL)
        {
            header->next = item;
            item->prev = header;
            item->next = NULL;
            return;
        }

        header = header->next;
    }

    item->next = header;
    item->prev = header->prev;

    if (header->prev != NULL)
    {
        header->prev->next = item;
    }
    header->prev = item;
}

static void remove_ordered_list(vheader_t *item)
{
    if (item == headers)
    {
        headers = NULL;
        return;
    }

    if (item->prev != NULL)
    {
        item->prev->next = item->next;
    }

    if (item->next != NULL)
    {
        item->next->prev = item->prev;
    }
}

static uint8_t set_heapinfo(void *adr, size_t value)
{
    size_t *heapinfo = (size_t *)get_heapinfo_pointer(adr);

    if (get_phys_adr(heapinfo) == NULL)
    {
        void *phys_page = alloc_page(MM_STANDART);
        if (phys_page == NULL)
        {
            return false;
        }
        map_page(alloc_page(MM_STANDART), get_page_aligned(heapinfo), PAGE_PRESENT | PAGE_WRITABLE);
    }

    *heapinfo = value;
    return true;
}

static void *vregion_alloc(size_t size)
{
    size = div_up(size, PAGE_SIZE) * PAGE_SIZE;

    vheader_t *header = headers;
    while (header->size < size)
    {
        if (header->next == NULL)
        {
            return NULL;
        }

        header = header->next;
    }

    remove_ordered_list(header);

    if (!set_heapinfo((void *)((size_t)header->region + size - PAGE_SIZE), (size_t)NULL))
    {
        return NULL;
    }
    if (!set_heapinfo(header->region, (size_t)size))
    {
        return NULL;
    }

    void *region = header->region;
    header->region = (uint8_t *)header->region +  size;
    header->size -= size;

    if (header->size != 0)
    {
        insert_ordered_list(header);

        if (!set_heapinfo(header->region, (size_t)header))
        {
            return NULL;
        }
        if (!set_heapinfo((uint8_t *)header->region + header->size - PAGE_SIZE, (size_t)header))
        {
            return NULL;
        }
    }

    return region;
}

static void vregion_free(void *region)
{
    size_t *heapinfo_start = (size_t *)get_heapinfo_pointer(region);
    size_t size = *heapinfo_start;
    size_t *heapinfo_end = (size_t *)get_heapinfo_pointer((size_t)region + size - PAGE_SIZE);

    size_t *heapinfo_prev = --heapinfo_start;
    size_t *heapinfo_next = ++heapinfo_end;

    vheader_t *new_header = (vheader_t *)kmalloc(sizeof(vheader_t), MM_STANDART);
    if (new_header == NULL)
    {
        // critical error, waiting for kpanic to be inserted
    }

    new_header->region = region;
    new_header->size = size;

    if ((size_t)region > VHEAP_START && *heapinfo_prev > KERNEL_REGION)
    {
        vheader_t *prev_header = (vheader_t *)*heapinfo_prev;

        remove_ordered_list(prev_header);

        new_header->region = prev_header->region;
        new_header->size += prev_header->size;

        kfree(prev_header);
    }

    if (((size_t)region + size) < VHEAP_MAX && *heapinfo_next > KERNEL_REGION)
    {
        vheader_t *next_header = (vheader_t *)*heapinfo_next;

        remove_ordered_list(next_header);

        new_header->size += next_header->size;

        kfree(next_header);
    }

    insert_ordered_list(new_header);

    set_heapinfo(new_header->region, (size_t)new_header);
    set_heapinfo((uint8_t *)new_header->region + new_header->size - PAGE_SIZE, (size_t)new_header);
}

void *vmalloc(size_t size, uint32_t flags)
{
    if (flags != MM_STANDART)
    {
        return NULL;
    }

    void *region = vregion_alloc(size);
    if (region == NULL)
    {
        return NULL;
    }

    for (void *i = region; (uint8_t *)i < (uint8_t *)region + size; i = (uint8_t *)i + PAGE_SIZE)
    {
        void *phys_page = alloc_page(MM_STANDART);
        if (phys_page == NULL)
        {
            return NULL;
        }
        map_page(phys_page, i, PAGE_PRESENT | PAGE_WRITABLE);
    }

    return region;
}

void vfree(void *obj)
{
    if ((obj == NULL) || (obj != (void *)((size_t)obj & (~0xFFFU))))
    {
        return;
    }

    size_t size = *(size_t *)get_heapinfo_pointer(obj);

    for (void *i = obj; (uint8_t *)i < (uint8_t *)obj + size; i = (uint8_t *)i + PAGE_SIZE)
    {
        free_page(get_phys_adr(i));
        unmap_page(i);
    }

    vregion_free(obj);

    return;
}

void *map_io(void *io_region, size_t size)
{
    void *region = vregion_alloc(size);
    if (region == NULL)
    {
        return NULL;
    }

    for (void *i = region; (uint8_t *)i < (uint8_t *)region + size; i = (uint8_t *)i + PAGE_SIZE)
    {
        map_page(io_region, i, PAGE_PRESENT | PAGE_WRITABLE);
        io_region = (uint8_t *)io_region +  PAGE_SIZE;
    }

    return region;
}

void unmap_io(void *obj)
{
    if (obj != (uint8_t *)((size_t)obj & (~0xFFFU)))
    {
        return;
    }

    size_t size = *(size_t *)get_heapinfo_pointer(obj);

    for (void *i = obj; (uint8_t *)i < (uint8_t *)obj + size; i = (uint8_t *)i + PAGE_SIZE)
    {
        unmap_page(i);
    }

    vregion_free(obj);

    return;
}

void init_vheap()
{
    vheader_t *init_header = kmalloc(sizeof(vheader_t), MM_STANDART);

    *init_header = (vheader_t){NULL, NULL, VHEAP_MAX - VHEAP_START, (void *)VHEAP_START};

    headers = init_header;

    set_heapinfo(init_header->region, (size_t)init_header);
    set_heapinfo((uint8_t *)init_header->region + init_header->size - PAGE_SIZE, (size_t)init_header);
}
