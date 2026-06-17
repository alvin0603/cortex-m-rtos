#ifndef KERNEL_SEMAPHORE_H
#define KERNEL_SEMAPHORE_H

#include "kernel/task.h"
#include "kernel/scheduler.h"
#include <stdint.h>

#define MAX_WAITING_TASKS 10

typedef struct
{
    uint32_t count;
    TCB* wait_queue[MAX_WAITING_TASKS];
    uint32_t waiting_count;
} Semaphore;

void sem_init(Semaphore *sem, uint32_t init_count);
void sem_wait(Semaphore *sem);
void sem_post(Semaphore *sem);

#endif