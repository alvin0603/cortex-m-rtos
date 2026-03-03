#include "kernel/scheduler.h"
#include "kernel/task.h"
#include <stdint.h>
#include <stddef.h>
#define MAX_TASKS 5
#define ICSR  (*((volatile uint32_t *)0xE000ED04))
#define PENDSVSET 0x10000000

TCB *current_task = NULL;
TCB *next_task = NULL;
TCB *task_list[MAX_TASKS];
uint32_t task_count = 0;   
uint32_t current_task_index = 0;

void scheduler_init(void)
{
    task_count = 0;
    current_task_index = 0;
}
void scheduler_add_task(TCB *tcb)
{
    task_list[task_count] = tcb;
    task_count++;
}
void scheduler_yield(void)
{
    // Round Robin
    next_task = task_list[(current_task_index + 1) % task_count];
    current_task_index++;

    // Trigger PendSV exception
    ICSR = PENDSVSET;
}
void scheduler_start(void)
{
    current_task = task_list[0];
    __asm volatile(
        "ldr r0, [%0] \n"              
        "ldmia r0!, {r4-r11} \n"        
        "msr psp, r0 \n"               
        "mov r0, #2 \n"                
        "msr control, r0 \n"
        "isb \n" // flush                        
        "pop {r0-r3, r12, lr} \n"       
        "pop {pc} \n"                   
        : 
        : "r" (&current_task->stack_pointer) 
        : "r0", "memory"
    );
    
}   