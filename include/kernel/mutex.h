#ifndef KERNEL_MUTEX_H
#define KERNEL_MUTEX_H

#include "kernel/task.h"
typedef struct
{
    volatile uint8_t locked; // 0 = free 1 = locked
    TCB *owner;
} Mutex;

void mutex_init(Mutex *mtx);
void mutex_lock(Mutex *mtx);
void mutex_unlock(Mutex *mtx);

#endif