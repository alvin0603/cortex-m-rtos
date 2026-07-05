#include "kernel/scheduler.h"
#include "kernel/task.h"
#include "kernel/software_timer.h"
#include "hal/uart.h"
#include <stdint.h>
#include <stddef.h>
#define MAX_TASKS 5
#define ICSR  (*((volatile uint32_t *)0xE000ED04))
#define PENDSVSET 0x10000000
#define SHPR3 (*((volatile uint32_t *)0xE000ED20))

TCB *current_task = NULL;
TCB *next_task = NULL;
TCB *task_list[MAX_TASKS];
uint32_t task_count = 0;   
uint32_t current_task_index = 0;

volatile uint32_t system_ticks = 0;

uint32_t scheduler_get_ticks(void)
{
    return system_ticks;
}

/* Scheduler API */
void scheduler_init(void)
{
    task_count = 0;
    current_task_index = 0;
    SHPR3 |= (0xFF << 16); // context switch set to the lowest priority ([31:24] -> Systick's priority [23:16] -> PendSV's priority)
}
void scheduler_add_task(TCB *tcb)
{
    task_list[task_count] = tcb;
    task_count++;
}
static void scheduler_select_next_task(void)
{
    uint32_t highest_priority = 1e9;
    uint32_t next_index = current_task_index; // Self in default
    int found = 0;
    uint32_t search_index = current_task_index;
    for (uint32_t i = 0; i < task_count; i++)
    {
        search_index = (search_index + 1) % task_count; 
        if (task_list[search_index]->sleep_count == 0 && 
            task_list[search_index]->priority < highest_priority &&
            task_list[search_index]->state == READY)
        {
            highest_priority = task_list[search_index]->priority;
            next_index = search_index;
            found = 1;
        }
    }
    if (found) 
    {
        next_task = task_list[next_index];
        current_task_index = next_index;
        ICSR = PENDSVSET;
    }
}
void scheduler_yield(void)
{
    scheduler_select_next_task();
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

void scheduler_print_tasks(void)
{
    uart_puts("ID\tPRIO\tSTATE\tSLEEP\n");
    uart_puts("---------------------------------\n");
    for (uint32_t i = 0; i < task_count; i++)
    {
        uart_print_num(i); 
        uart_puts("\t");
        uart_print_num(task_list[i]->priority); 
        uart_puts("\t"); 
        if (task_list[i] == current_task) 
            uart_puts("RUNNING");
        else 
        {
            switch (task_list[i]->state) 
            {
                case READY:   uart_puts("READY"); break;
                case BLOCKED: uart_puts("BLOCK"); break;
                default:      uart_puts("UNKNOWN"); break;
            }
        }
        uart_puts("\t");
        uart_print_num(task_list[i]->sleep_count); 
        uart_puts("\n");
    }
}

/* SysTick */
void SysTick_Handler(void)
{
    system_ticks++;
    // Decrement sleep counters for all sleeping tasks
    for (uint32_t i = 0; i < task_count; i++)
    {
        if (task_list[i]->sleep_count > 0)
            task_list[i]->sleep_count--;
    }
    timer_system_tick();
    scheduler_select_next_task();
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