#include "kernel/task.h"    
#include <stdint.h>
#include <stddef.h>
void task_create(TCB *tcb, void (*task_function)(void), uint32_t *stack, uint32_t priority)
{
    for(uint32_t i = 0; i < TASK_STACK_SIZE; i++)
        stack[i] = 0xDEADBEEF; // unused
    tcb->stack_base = stack;
    // ARM stack use Full Descending
    uint32_t *sp = stack + TASK_STACK_SIZE;
    *(--sp) = 0x01000000; // xPSR thumb bit = 1
    *(--sp) = (uint32_t)task_function; // Entry Point for Task
    *(--sp) = 0xFFFFFFFD; // LR
    *(--sp) = 0x00000000; // R12
    *(--sp) = 0x00000000; // R3
    *(--sp) = 0x00000000; // R2
    *(--sp) = 0x00000000; // R1
    *(--sp) = 0x00000000; // R0
    for(int i = 0; i < 8; i++)
        *(--sp) = 0x00000000; // R4-R11
    tcb->stack_pointer = sp;
    tcb->sleep_count = 0;
    tcb->priority = priority;
    tcb->original_priority = priority;
    tcb->run_ticks = 0;
    tcb->state = READY;
    tcb->next = NULL;
}

uint32_t task_get_stack_used(TCB *tcb)
{
    uint32_t unused = 0;
    for(uint32_t i = 0; i < TASK_STACK_SIZE; i++)
    {
        if(tcb->stack_base[i] == 0xDEADBEEF)
            unused++;
        else
            break;
    }
    return TASK_STACK_SIZE - unused;
}