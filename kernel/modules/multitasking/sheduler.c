#include <stddef.h>

#include <config.h>
#include <hal.h>
#include <multitasking.h>
#include <linked_list.h>
#include <debug.h>
#include <kprintf.h>

#include "mtdef.h"

extern void switch_task(task_t *next_task);

static bool postponed_task_switch = false;

void yield(void)
{
    current_task->state = READY;
    push_back(current_task, &ready_tasks);
    schedule();
}

void block_task(task_state reason)
{
#ifdef DEBUG_MT
    dbg_kprintf("\n%s call(reason: %d)\n", __func__, reason);
#endif

    current_task->state = reason;
    schedule();
}

void unblock_task(task_t *task)
{
#ifdef DEBUG_MT
    dbg_kprintf("\n%s call(task: %p)\n", __func__, task);
#endif

    task->state = READY;
    push_back(task, &ready_tasks);

    schedule();
}

void schedule()
{
    // Switch to real task is already postponed
    if(postponed_task_switch) return;

    postponed_task_switch = true;
    while(ready_tasks == NULL)
    {
        // All task are blocked so just postpone task switch 
        // and wait until an interrupt will unblock some task

        sti();
        hlt();
        cli();
    }
    postponed_task_switch = false;

    task_t *next_task = pop_front(&ready_tasks);
    
    next_task->state = WORKING;

    switch_task(next_task);
}