#ifndef MULTITASKING_H
#define MULTITASKING_H

#include <stddef.h>
#include <stdint.h>
#include <linked_list.h>

typedef enum task_state
{
    WORKING = 0,
    READY = 1,
    SLEEP = 2,
    WAITING = 3,
    TERMINATED = 4
} task_state;

typedef struct task_t task_t;
struct task_t
{
    void *kernel_stack;
    void *page_directory;

    uint32_t pid;
    task_state state;
};

#define TICK_HZ 100

uint32_t nanoseconds_since_boot;

task_t *create_task(void launch_function(void));

void terminate_task(task_t *task);

void yield(void);

void block_task(task_state reason);

void unblock_task(task_t *task);

void nanosleep_until(uint32_t time);

static inline void nanosleep(uint32_t nanoseconds)
{
    nanosleep_until(nanoseconds_since_boot + nanoseconds);
}

static inline void sleep(uint32_t seconds)
{
    uint32_t wait_time = seconds * 1000000;
    nanosleep_until(nanoseconds_since_boot + wait_time);
}

void init_multitasking(void);

#endif /* MULTITASKING_H */