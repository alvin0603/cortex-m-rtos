#include "kernel/scheduler.h"
#include "hal/uart.h"
#include "kernel/software_timer.h"
#include "kernel/task.h"
#include <stddef.h>
#include <stdint.h>

#define MAX_TASKS       10
#define MAX_PRIORITY    32
#define ICSR            (*((volatile uint32_t *)0xE000ED04))
#define PENDSVSET       0x10000000
#define SHPR3           (*((volatile uint32_t *)0xE000ED20))

uint32_t ready_bitmap = 0;
TCB *ready_list[MAX_PRIORITY] = {NULL};

TCB *task_list[MAX_TASKS];
TCB *current_task = NULL;
TCB *next_task = NULL;
uint32_t task_count = 0;

uint32_t context_switch_count = 0;
volatile uint32_t system_ticks = 0;

uint32_t scheduler_get_ticks(void) 
{ 
    return system_ticks; 
}

uint32_t scheduler_get_context_switches(void) 
{ 
    return context_switch_count; 
}

/* Scheduler API */
void scheduler_init(void) 
{
    task_count = 0;
    SHPR3 |= (0xFF << 16); // context switch set to the lowest priority ([31:24] -> Systick's priority [23:16] -> PendSV's priority)
}

void scheduler_add_task(TCB *tcb) 
{
    task_list[task_count] = tcb;
    task_count++;
    add_to_ready_list(tcb);
}

void add_to_ready_list(TCB *task) 
{
    task->next = NULL;
    if(ready_list[task->priority] == NULL) 
    {
        ready_list[task->priority] = task;
        ready_bitmap |= (1u << task->priority);
    } 
    else 
    {
        TCB *tmp = ready_list[task->priority];
        while(tmp->next != NULL)
            tmp = tmp->next;
        tmp->next = task;
    }
}

TCB *pop_from_ready_list(uint32_t priority) 
{
    TCB *task = ready_list[priority];
    if(task != NULL) 
    {
        ready_list[priority] = task->next;
        if(ready_list[priority] == NULL)
            ready_bitmap &= ~(1u << priority);
    }
    return task;
}

void remove_from_ready_list(TCB *task) 
{
    if(ready_list[task->priority] == NULL) 
        return;
    else if(ready_list[task->priority] == task) 
    {
        ready_list[task->priority] = task->next;
        if(ready_list[task->priority] == NULL)
            ready_bitmap &= ~(1u << task->priority);
    } 
    else 
    {
        TCB *tmp = ready_list[task->priority];
        while(tmp->next != NULL && tmp->next != task)
            tmp = tmp->next;
        if(tmp->next == task)
            tmp->next = task->next;
    }
}

void scheduler_set_priority(TCB *task, uint32_t new_priority) // for Mutex PIP usage
{
    if(task->state == READY && task->sleep_count == 0 && task != current_task)
        remove_from_ready_list(task);
    
    task->priority = new_priority;
    
    if(task->state == READY && task->sleep_count == 0 && task != current_task)
        add_to_ready_list(task);
}

static void scheduler_select_next_task(void) 
{
    critical_enter();
    
    if(current_task != NULL && current_task->state == READY && current_task->sleep_count == 0)
        add_to_ready_list(current_task);

    TCB *chosen_task = current_task;
    if(ready_bitmap != 0) 
    {
        uint32_t highest_priority = __builtin_ctz(ready_bitmap);
        chosen_task = pop_from_ready_list(highest_priority);
    }
    
    critical_exit();

    if(chosen_task != NULL && chosen_task != current_task) 
    {
        context_switch_count++;
        next_task = chosen_task;
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
    uint32_t highest_priority = __builtin_ctz(ready_bitmap);
    current_task = pop_from_ready_list(highest_priority);

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
        : "r"(&current_task->stack_pointer)
        : "r0", "memory"
    );
}

void scheduler_print_tasks(void) 
{
    uart_puts("ID\tPRIO\tSTATE\tSLEEP\tCPU%\n");
    uart_puts("---------------------------------\n");
    for(uint32_t i = 0; i < task_count; i++) 
    {
        uart_print_num(i);
        uart_puts("\t");
        uart_print_num(task_list[i]->priority);
        uart_puts("\t");
        
        if(task_list[i] == current_task)
            uart_puts("RUNNING");
        else 
        {
            switch(task_list[i]->state) 
            {
                case READY:
                    uart_puts("READY");
                    break;
                case BLOCKED:
                    uart_puts("BLOCK");
                    break;
                default:
                    uart_puts("UNKNOWN");
                    break;
            }
        }
        uart_puts("\t");
        uart_print_num(task_list[i]->sleep_count);
        uart_puts("\t");
        uint32_t cpu_percent = 0;
        if(system_ticks > 0)
            cpu_percent = (task_list[i]->run_ticks * 100) / system_ticks;
        uart_print_num(cpu_percent);
        uart_puts(" %\n");
    }
    uart_puts("---------------------------------\n");
}

void scheduler_print_stacks(void) 
{
    uart_puts("Task ID\tUsed\tTotal\n");
    uart_puts("------------------------\n");
    for(uint32_t i = 0; i < task_count; i++) 
    {
        uart_print_num(i);
        uart_puts("\t");
        uart_print_num(task_get_stack_used(task_list[i]));
        uart_puts("\t");
        uart_print_num(TASK_STACK_SIZE);
        uart_puts("\n");
    }
}

/* SysTick */
void SysTick_Handler(void) 
{
    system_ticks++;
    current_task->run_ticks++;
    
    // Decrement sleep counters for all sleeping tasks
    for(uint32_t i = 0; i < task_count; i++) 
    {
        if(task_list[i]->sleep_count > 0) 
        {
            task_list[i]->sleep_count--;
            if(task_list[i]->sleep_count == 0 && task_list[i]->state == READY)
                add_to_ready_list(task_list[i]);
        }
    }
    timer_system_tick();
    scheduler_select_next_task();
}

/* Critical Section */
void critical_enter(void) 
{
    __asm volatile("cpsid i"); // Disable interrupt
}

void critical_exit(void) 
{
    __asm volatile("cpsie i"); // Enable interrupt
}