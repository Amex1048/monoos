#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <config.h>
#include <memory.h>
#include <linked_list.h>
#include <kprintf.h>
#include <debug.h>

#include <multitasking.h>
#include "mtdef.h"

typedef struct sleep_task_t sleep_task_t;
struct sleep_task_t
{
    uint32_t nanoseconds_awake;
    task_t *task;
};

static bool add_comparator(void *obj_prev, void *obj_add, void *obj_next)
{
    sleep_task_t *sleep_add = (sleep_task_t *)obj_add;
    sleep_task_t *sleep_next = (sleep_task_t *)obj_next;

    if (sleep_add->nanoseconds_awake <= sleep_next->nanoseconds_awake)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void nanosleep_until(uint32_t time)
{
#ifdef DEBUG_MT
    dbg_kprintf("\n%s call(time: %d)\n", __func__, time);
    dbg_kprintf("Time now: %d\n", nanoseconds_since_boot);
#endif

    if (time <= nanoseconds_since_boot) return;

    sleep_task_t *sleep_task = kmalloc(sizeof(sleep_task_t), MM_STANDART);
    sleep_task->task = current_task;
    sleep_task->nanoseconds_awake = time;

    insert_with_compare(sleep_task, &sleep_tasks, add_comparator);

    block_task(SLEEP);
}

task_t *get_awaked_task(void)
{
    if (sleep_tasks == NULL)
    {
        return NULL;
    }

    sleep_task_t *next_task = sleep_tasks->obj;

    if (next_task->nanoseconds_awake <= nanoseconds_since_boot) 
    {
        return ((sleep_task_t *)pop_front(&sleep_tasks))->task;
    }
    else return NULL;
}