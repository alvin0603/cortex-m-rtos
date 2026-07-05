#ifndef KERNEL_MUTEX_H
#define KERNEL_MUTEX_H

#include "kernel/task.h"

#define MAX_WAITING_TASKS 10

typedef struct
{
    volatile uint8_t locked; // 0 = free 1 = locked
    TCB *owner;
    TCB *wait_queue[MAX_WAITING_TASKS];
    uint32_t waiting_count;
} Mutex;

void mutex_init(Mutex *mtx);
void mutex_lock(Mutex *mtx);
void mutex_unlock(Mutex *mtx);

#endif