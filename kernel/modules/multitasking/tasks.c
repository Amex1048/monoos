#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <config.h>
#include <memory.h>
#include <string.h>
#include <multitasking.h>
#include <linked_list.h>
#include <hal.h>
#include <kprintf.h>

#include <debug.h>

#include "mtdef.h"

#define TASK_INIT_STACK_OFFSET (4 * 5)

typedef struct tss_descriptor
{
    uint16_t limit;
    uint16_t base0_15;
    uint8_t base16_23;
    uint16_t flags;
    uint8_t base24_31;
} __attribute__((packed)) tss_descriptor;

extern uint32_t tss;
extern uint32_t tss_entry;

extern void init_tss(void);

task_t *create_task(void launch_function(void))
{
#ifdef DEBUG_MT
    dbg_kprintf("\n%s (launch_function: %p)", __func__, launch_function);
#endif

    task_t *new_task = kmalloc(sizeof(task_t), MM_STANDART);

    if (new_task == NULL)
    {
        return NULL;
    }

    void *new_stack = kmalloc(PAGE_SIZE, MM_STANDART);
    void *new_pade_dir = alloc_page(MM_STANDART);

    if ((new_stack == NULL) | (new_pade_dir == NULL))
    {
        return NULL;
    }

    *(size_t *)((uint8_t *)new_stack + PAGE_SIZE - 1 - sizeof(size_t)) = (size_t)launch_function;

    new_task->kernel_stack = (uint8_t *)new_stack + PAGE_SIZE - 1 - TASK_INIT_STACK_OFFSET;
    new_task->page_directory = new_pade_dir;

    void *page_dir_map = map_io(new_pade_dir, PAGE_SIZE);

    // Create empty table directory and mount kernel address space
    memset(page_dir_map, 0, ((PAGE_SIZE / 8) * 7));

    memcpy((uint8_t *)page_dir_map + ((PAGE_SIZE / 8) * 7),
           (uint8_t *)0xFFFFF000U + ((PAGE_SIZE / 8) * 7),
           PAGE_SIZE / 8);

    unmap_io(page_dir_map);

    new_task->state = READY;
    push_back(new_task, &ready_tasks);

#ifdef DEBUG_MT
    dbg_kprintf("Task created: %p", new_task);
#endif

    return new_task;
}

void terminate_task(task_t *task)
{
#ifdef DEBUG_MT
    dbg_kprintf("\n%s call (task: %p)", __func__, task);
#endif
    task->state = TERMINATED;
    push_back(task, &terminated_tasks);

    schedule();

    // TODO free terminated task memory via special thread
}

static void test_sleep(void)
{
    sleep(3);
    kprintf("sleep(3) 1\n");

    sleep(3);

    kprintf("sleep(3) 2\n");

    sleep(3);

    kprintf("sleep(3) 3\n");

    cli();
    hlt();
}

void init_multitasking(void)
{
    {
        uint32_t tss_adress = (uint32_t)&tss;
        tss_descriptor *desc = (tss_descriptor *)&tss_entry;
        desc->limit = 0xFFFF;
        desc->base0_15 = (uint16_t)(tss_adress & 0xFFFF);
        desc->base16_23 = (uint8_t)((tss_adress >> 16) & 0xFF);
        desc->flags = 0xCF89;
        desc->base24_31 = (uint8_t)(tss_adress >> 24);

        init_tss();
    }

    init_timer();

    task_t *first_task = kmalloc(sizeof(task_t), MM_STANDART);

    __asm__ volatile("movl %%esp, %0"
                     : "=a"(first_task->kernel_stack)
                     :);
    __asm__ volatile("movl %%cr3, %0"
                     : "=a"(first_task->page_directory)
                     :);

    current_task = first_task;
    first_task->state = WORKING;

    create_task(test_sleep);
    block_task(READY);
}
