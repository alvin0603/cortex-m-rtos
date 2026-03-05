#ifndef KERNEL_SCHEDULER_H
#define KERNEL_SCHEDULER_H

#include "kernel/task.h"

extern TCB *current_task;
void scheduler_init(void);
void scheduler_start(void);
void scheduler_yield(void);
void scheduler_add_task(TCB *tcb);
void task_sleep(uint32_t ticks);
void critical_enter(void);
void critical_exit(void);

#endif