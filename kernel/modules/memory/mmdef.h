#ifndef MMDEF_H
#define MMDEF_H

#include <stddef.h>

#include <boot.h>

#define PHYS_MEM_LIMIT 0x1000000U
#define KERNEL_REGION 0xE0000000U

#define HEAP_INFO_REGION (0xE1000000U - 0x80000U)
#define HEAP_INFO_REGION_SIZE 0x80000U

#define get_heapinfo_pointer(pointer) (void **)(HEAP_INFO_REGION + (((size_t)(pointer)-KERNEL_REGION) / 1024))
#define get_page_aligned(adress) (void *)((size_t)(adress) & (~0xFFFU))

uint32_t available_mem;

// PMM
void init_pmm(mem_map *boot);

// HEAP

void init_kheap(void);
void init_vheap(void);

#endif /* MMDEF_H */
