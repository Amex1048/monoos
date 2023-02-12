#ifndef MTDEF_H
#define MTDEF_H

#pragma once

#include <stddef.h>

#include <multitasking.h>
#include <linked_list.h>

list_t *ready_tasks;
list_t *terminated_tasks;

task_t *current_task;

list_t *sleep_tasks;

void print_ready(void);

void init_timer(void);

task_t *get_awaked_task(void);

void schedule(void);

#endif /* MTDEF_H */