#include "kernel/task.h"    
void task_create(TCB *tcb, void (*task_function)(void), uint32_t *stack)
{
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
}