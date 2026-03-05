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

/* Scheduler API */
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
    // Round Robin: find next awake task
    uint32_t next_index = current_task_index;
    for (uint32_t i = 0; i < task_count; i++)
    {
        next_index = (next_index + 1) % task_count;
        if (task_list[next_index]->sleep_count == 0)
        {
            next_task = task_list[next_index];
            current_task_index = next_index;
            ICSR = PENDSVSET;
            return;
        }
    }
    // All tasks sleeping: stay on current task
}

void task_sleep(uint32_t ticks)
{
    current_task->sleep_count = ticks;
    scheduler_yield();
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

/* SysTick */
void SysTick_Handler(void)
{
    // Decrement sleep counters for all sleeping tasks
    for (uint32_t i = 0; i < task_count; i++)
    {
        if (task_list[i]->sleep_count > 0)
            task_list[i]->sleep_count--;
    }

    // Find next awake task
    uint32_t next_index = current_task_index;
    for (uint32_t i = 0; i < task_count; i++)
    {
        next_index = (next_index + 1) % task_count;
        if (task_list[next_index]->sleep_count == 0)
        {
            next_task = task_list[next_index];
            current_task_index = next_index;
            ICSR = PENDSVSET;
            return;
        }
    }
    // All tasks sleeping: no context switch
}

/* Critical Section */
void critical_enter(void)
{
    __asm volatile(
        "cpsid i"    
    ); // Disable interrupt
}
void critical_exit(void)
{
    __asm volatile(
        "cpsie i"
    ); // Enable interrupt
}