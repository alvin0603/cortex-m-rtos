// Entry point for Mini RTOS

#include "hal/uart.h"
#include "kernel/task.h"
#include "kernel/scheduler.h"
#include "hal/systick.h"

uint32_t task_1_stack[TASK_STACK_SIZE];
uint32_t task_2_stack[TASK_STACK_SIZE];
TCB tcb1,tcb2;
void task1(void)
{
    while(1)
    {
        uart_puts("Task 1 is running\n");
        task_sleep(100); // sleep 1 second (100 ticks * 10ms)
    }
}
void task2(void)
{
    while(1)
    {
        uart_puts("Task 2 is running\n");
        task_sleep(50); // sleep 0.5 second (50 ticks * 10ms)
    }
}

int main(void)
{
    uart_init();

    scheduler_init();
    task_create(&tcb1, task1, task_1_stack);
    task_create(&tcb2, task2, task_2_stack);
    scheduler_add_task(&tcb1);
    scheduler_add_task(&tcb2);
    systick_init(120000);
    scheduler_start();

    // lock CPU
    while (1){}
    return 0; // never reach
}
