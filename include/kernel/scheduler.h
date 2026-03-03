#ifndef KERNEL_SCHEDULER_H
#define KERNEL_SCHEDULER_H

#include "kernel/task.h"

void scheduler_init(void);
void scheduler_start(void);
void scheduler_yield(void);
void scheduler_add_task(TCB *tcb);
void task_sleep(uint32_t ticks);

#endif