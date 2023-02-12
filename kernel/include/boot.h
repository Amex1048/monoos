#ifndef BOOT_H
#define BOOT_H

#include <stddef.h>
#include <stdint.h>

// Memory map for physical memory manager initialization
typedef struct mem_map
{
    uint16_t low_mem;
    uint16_t low_mem_reserved;
    uint16_t high_mem;
    uint16_t high_mem_reserved;
} mem_map;

typedef struct boot_info
{
    uint32_t kernel_size;
    mem_map map;
} boot_info;

#endif /* BOOT_H */
