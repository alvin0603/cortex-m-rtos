// Entry point for Mini RTOS

#include "hal/uart.h"
#include "kernel/task.h"
#include "kernel/scheduler.h"
#include "hal/systick.h"
#include "kernel/message_queue.h"

uint32_t task_1_stack[TASK_STACK_SIZE];
uint32_t task_2_stack[TASK_STACK_SIZE];
uint32_t idle_task_stack[TASK_STACK_SIZE];
TCB tcb1,tcb2;
TCB idle_tcb;

#define MQ_CAPACITY 5
uint32_t mq_buffer[MQ_CAPACITY];
MessageQueue my_mq;

void task1(void)
{
    uint32_t send_val = 0;
    while(1)
    {
        msg_queue_send(&my_mq, send_val);
        uart_puts("Producer sent: ");
        uart_putc('0' + (send_val % 10));
        uart_puts("\n");
        send_val++;
        task_sleep(10);
    }
}
void task2(void)
{
    uint32_t recv_val;
    while(1)
    {
        msg_queue_receive(&my_mq, &recv_val);
        uart_puts("  Consumer got: ");
        uart_putc('0' + (recv_val % 10));
        uart_puts("\n");
        task_sleep(100);
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
    msg_queue_init(&my_mq, mq_buffer, MQ_CAPACITY);

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
