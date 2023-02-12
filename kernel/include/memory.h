#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

#include <boot.h>

#define PAGE_SIZE 0x1000U

#define PAGE_PRESENT (1 << 0)
#define PAGE_WRITABLE (1 << 1)
#define PAGE_USER (1 << 2)

#define MM_STANDART 0
#define MM_DMA (1 << 0)

// VMM
void init_mm(mem_map *map);

void *alloc_pages(uint32_t count, uint32_t flags);
void *alloc_page(uint32_t flags);

void free_pages(void *pages, uint32_t count);
void free_page(void *page);

void *get_phys_adr(void *virtual_adr);

void map_page(void *phys_adr, void *virtual_adr, uint32_t flags);
void unmap_page(void *virtual_adr);

void *kmalloc(size_t size, uint32_t flags);
void kfree(void *obj);

void *vmalloc(size_t size, uint32_t flags);
void vfree(void *obj);

void *map_io(void *io_region, size_t size);
void unmap_io(void *obj);

void print_caches(void);

#endif /* MEMORY_H */