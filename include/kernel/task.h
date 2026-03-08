#ifndef KERNEL_TASK_H
#define KERNEL_TASK_H

#include <stdint.h>
#define TASK_STACK_SIZE 256

// task control block
typedef struct
{
    uint32_t *stack_pointer;
    uint32_t sleep_count;
    uint32_t priority;
} TCB; 

void task_create(TCB *tcb, void (*task_function)(void), uint32_t *stack, uint32_t priority);

#endif