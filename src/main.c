// Entry point for Mini RTOS

#include "hal/systick.h"
#include "hal/uart.h"
#include "kernel/message_queue.h"
#include "kernel/scheduler.h"
#include "kernel/software_timer.h"
#include "kernel/task.h"


uint32_t task_1_stack[TASK_STACK_SIZE];
uint32_t task_2_stack[TASK_STACK_SIZE];
uint32_t idle_task_stack[TASK_STACK_SIZE];
TCB tcb1, tcb2;
TCB idle_tcb;

#define MQ_CAPACITY 5
uint32_t mq_buffer[MQ_CAPACITY];
MessageQueue my_mq;

void uart_print_num(uint32_t num) 
{
    if (num == 0) 
    {
        uart_putc('0');
        return;
    }
    char buf[10];
    int i = 0;
    while (num > 0) 
    {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }
    while (i > 0)
        uart_putc(buf[--i]);
}

void print_timestamp(void)
{
    uart_print_num(scheduler_get_ticks());
    uart_puts(":");
}

softwareTimer periodic_timer;
softwareTimer oneshot_timer;

void periodic_callback(void) 
{ 
    print_timestamp();
    uart_puts("Periodic timer fired\n"); 
}

void oneshot_callback(void) 
{ 
    print_timestamp();
    uart_puts("One-shot timer fired\n"); 
}

void task1(void) 
{
  uint32_t send_val = 0;
  while (1) 
  {
    print_timestamp();
    msg_queue_send(&my_mq, send_val);
    uart_puts("Producer sent: ");
    uart_print_num(send_val);
    uart_puts("\n");
    send_val++;
    task_sleep(10);
  }
}
void task2(void) 
{
  uint32_t recv_val;
  while (1) 
  {
    print_timestamp();
    msg_queue_receive(&my_mq, &recv_val);
    uart_puts("  Consumer got: ");
    uart_print_num(recv_val);
    uart_puts("\n");
    task_sleep(100);
  }
}
void idle_task(void) 
{
  while (1) 
  {
    __asm volatile("wfi"); // Wait for interrupt until next SysTick
  }
}
int main(void) {
  uart_init();
  msg_queue_init(&my_mq, mq_buffer, MQ_CAPACITY);

  timer_create(&periodic_timer, 50, 1, periodic_callback);
  timer_start(&periodic_timer);
  timer_create(&oneshot_timer, 200, 0, oneshot_callback);
  timer_start(&oneshot_timer);

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
  while (1) {
  }
  return 0; // never reach
}
