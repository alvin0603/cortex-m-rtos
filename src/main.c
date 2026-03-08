// Entry point for Mini RTOS

#include "hal/uart.h"
#include "kernel/task.h"
#include "kernel/scheduler.h"
#include "hal/systick.h"
#include "kernel/mutex.h"

uint32_t task_1_stack[TASK_STACK_SIZE];
uint32_t task_2_stack[TASK_STACK_SIZE];
uint32_t idle_task_stack[TASK_STACK_SIZE];
TCB tcb1,tcb2;
TCB idle_tcb;
Mutex uart_mutex;
uint32_t shared_counter = 0;

void task1(void)
{
    while(1)
    {
        mutex_lock(&uart_mutex);
        shared_counter++;
        uart_puts("Task 1 | counter = ");
        uart_putc('0' + (shared_counter % 10));
        uart_puts("\n");
        mutex_unlock(&uart_mutex);
        task_sleep(100);
    }
}
void task2(void)
{
    while(1)
    {
        mutex_lock(&uart_mutex);
        shared_counter++;
        uart_puts("Task 2 | counter = ");
        uart_putc('0' + (shared_counter % 10));
        uart_puts("\n");
        mutex_unlock(&uart_mutex);
        task_sleep(50);
    }
}
void idle_task(void)
{
    while(1)
    {
        uart_puts("Idle...\n");
        __asm volatile("wfi"); // Wait for interrupt until next SysTick
     }
}
int main(void)
{
    uart_init();

    scheduler_init();
    task_create(&tcb1, task1, task_1_stack, 0);
    task_create(&tcb2, task2, task_2_stack, 1);
    task_create(&idle_tcb, idle_task, idle_task_stack, 255);
    scheduler_add_task(&tcb1);
    scheduler_add_task(&tcb2);
    scheduler_add_task(&idle_tcb);
    systick_init(120000);
    scheduler_start();

    // lock CPU
    while (1){}
    return 0; // never reach
}
