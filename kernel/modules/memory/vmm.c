#include <stddef.h>
#include <stdint.h>

#include <config.h>
#include <string.h>
#include <memory.h>
#include <boot.h>

#include "mmdef.h"

#ifdef i386
#define clear_tlb(virtual_adr) __asm__ volatile("movl %cr3, %eax; \
												 movl %eax, %cr3");
#else
//TODO
#endif

#define PAGE_DIR_POS 0xFFFFF000

#define ADRRESS_MASK 0xFFFFF000
#define TABLE_ENTRY_MASK 0x3FF000

#define PAGE_TABLE_SIZE 0x1000
#define PAGE_TABLES_POS 0xFFC00000

#define IO_MAP_START 0xF4000000
#define IO_MAP_END 0xFBFFD000
#define IO_MAP_SIZE (IO_MAP_END - IO_MAP_START)
#define IO_MAP_BITMAP_SIZE div_up(div_up(IO_MAP_SIZE, PAGE_SIZE), 8)

#define HEAP_START 0xE1000000
#define HEAP_END 0xF4000000
#define HEAP_SIZE (HEAP_END - HEAP_START)

uint16_t mount_table[2048];
uint32_t *page_dir = (uint32_t *)PAGE_DIR_POS;

void *get_phys_adr(void *virtual_adr)
{
	uint16_t page_dir_index = (uint16_t)((size_t)virtual_adr >> 22);

	if ((page_dir[page_dir_index] & PAGE_PRESENT) == 0)
	{
		return NULL;
	}

	uint32_t *page_table = (uint32_t *)(PAGE_TABLES_POS + (PAGE_TABLE_SIZE * page_dir_index));
	uint16_t page_table_index = ((size_t)virtual_adr & TABLE_ENTRY_MASK) >> 12;

	if ((page_table[page_table_index] & PAGE_PRESENT) == 0)
	{
		return NULL;
	}

	return (void *)((page_table[page_table_index] & (~0xFFFU)) + ((size_t)virtual_adr & 0xFFFU));
}

void map_page(void *phys_adr, void *virtual_adr, uint32_t flags)
{
	uint16_t page_dir_index = (uint16_t)((size_t)virtual_adr >> 22);
	uint32_t *page_table = (uint32_t *)(PAGE_TABLES_POS + (PAGE_TABLE_SIZE * page_dir_index));

	if ((page_dir[page_dir_index] & PAGE_PRESENT) == 0)
	{
		void *new_page = alloc_page(0);
		if (new_page == NULL)
		{
			// critical exeption, waiting for kpanic to be inserted
		}

		page_dir[page_dir_index] = (uint32_t)new_page | PAGE_PRESENT | PAGE_WRITABLE;
		memset(page_table, 0, 4096);
	}
	uint16_t page_table_index = ((size_t)virtual_adr & TABLE_ENTRY_MASK) >> 12;

	page_table[page_table_index] = (size_t)phys_adr | (flags & 0xFFF);
	++mount_table[page_dir_index];
}

void unmap_page(void *virtual_adr)
{
	uint16_t page_dir_index = (uint16_t)((size_t)virtual_adr >> 22);

	uint32_t *page_table = (uint32_t *)(PAGE_TABLES_POS + (PAGE_TABLE_SIZE * page_dir_index));
	uint16_t page_table_index = ((size_t)virtual_adr & TABLE_ENTRY_MASK) >> 12;
	page_table[page_table_index] = 0;

	--mount_table[page_dir_index];

	if (mount_table[page_dir_index] == 0)
	{
		free_page(get_phys_adr(page_table));
		page_dir[page_dir_index] = 0;
	}

	clear_tlb(0);
}

static void unmap_dir(void *virtual_adr)
{
	uint16_t page_dir_index = (uint16_t)((size_t)virtual_adr >> 22);
	uint32_t *page_table = (uint32_t *)(PAGE_TABLES_POS + (PAGE_TABLE_SIZE * page_dir_index));

	free_page(get_phys_adr(page_table));

	page_dir[page_dir_index] = 0;
	mount_table[page_dir_index] = 0;

	clear_tlb(0);
}

void init_mm(mem_map *map)
{
	init_pmm(map);
	unmap_dir(0);

	for (uint16_t i = 0; i < 1024; ++i)
	{
		if (page_dir[i] & PAGE_PRESENT)
		{
			uint32_t *page_table = (uint32_t *)(PAGE_TABLES_POS + (PAGE_TABLE_SIZE * i));
			for (uint16_t j = 0; j < 1024; ++j)
			{
				if (page_table[j] & PAGE_PRESENT)
				{
					++mount_table[i];
				}
			}
		}
	}

	init_kheap();
	init_vheap();
}
