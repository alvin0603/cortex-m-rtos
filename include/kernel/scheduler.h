#ifndef KERNEL_SCHEDULER_H
#define KERNEL_SCHEDULER_H

#include "kernel/task.h"
#include <stdint.h>

extern TCB *current_task;
void scheduler_init(void);
void scheduler_start(void);
void scheduler_yield(void);
void scheduler_add_task(TCB *tcb);
void scheduler_print_tasks(void);
void scheduler_print_stacks(void);
uint32_t scheduler_get_context_switches(void);
uint32_t scheduler_get_ticks(void);
void task_sleep(uint32_t ticks);
void critical_enter(void);
void critical_exit(void);

#endif