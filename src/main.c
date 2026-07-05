// Entry point for Mini RTOS

#include "hal/systick.h"
#include "hal/uart.h"
#include "kernel/message_queue.h"
#include "kernel/scheduler.h"
#include "kernel/software_timer.h"
#include "kernel/task.h"
#include "kernel/memory_pool.h"
#include "kernel/mutex.h"

uint32_t high_stack[TASK_STACK_SIZE];
uint32_t medium_stack[TASK_STACK_SIZE];
uint32_t low_stack[TASK_STACK_SIZE];
uint32_t idle_task_stack[TASK_STACK_SIZE];
TCB high_tcb, medium_tcb, low_tcb;
TCB idle_tcb;
Mutex demo_mutex;

#define MQ_CAPACITY 5
uint32_t mq_buffer[MQ_CAPACITY];
MessageQueue my_mq;
MessageQueue shell_rx_queue;
uint32_t shell_rx_buffer[64];

uint32_t shell_task_stack[TASK_STACK_SIZE];
TCB shell_tcb;

#define BLOCK_SIZE 32
#define TOTAL_BLOCKS 8
uint32_t my_pool_buffer[(BLOCK_SIZE * TOTAL_BLOCKS) / 4];
MemoryPool my_pool;

typedef struct 
{
    uint32_t id;
    uint32_t data;
} SensorData;

void print_timestamp(void)
{
    uart_print_num(scheduler_get_ticks());
    uart_puts(":");
}

softwareTimer periodic_timer;
softwareTimer oneshot_timer;

void periodic_callback(void) 
{ 
    // print_timestamp();
    // uart_puts("Periodic timer fired\n"); 
}

void oneshot_callback(void) 
{ 
    // print_timestamp();
    // uart_puts("One-shot timer fired\n"); 
}

void task_low(void) 
{
    uart_puts("[Low] Started. Acquiring Mutex...\n");
    mutex_lock(&demo_mutex);
    uart_puts("[Low] Mutex Acquired! Doing slow work...\n");
    
    // Busy-loop
    for(volatile uint32_t i=0; i<3000000; i++); 
    
    uart_puts("[Low] Work done. Releasing Mutex...\n");
    mutex_unlock(&demo_mutex);
    uart_puts("[Low] Finished.\n");
    
    while(1)
    {
        uart_puts("[Low] Working...\n");
        task_sleep(1000);
    }
}

void task_high(void) 
{
    task_sleep(10);
    uart_puts("[High] Woke up! Needing Mutex...\n");
    mutex_lock(&demo_mutex);
    uart_puts("[High] Mutex Acquired! Doing critical work...\n");
    
    for(volatile uint32_t i=0; i<1000000; i++);
    
    mutex_unlock(&demo_mutex);
    uart_puts("[High] Finished.\n");
    
    while(1) 
      task_sleep(1000);
}

void task_medium(void) 
{
    task_sleep(20);
    uart_puts("[Medium] Woke up! I don't need Mutex, but I need CPU!\n");
    
    for(volatile uint32_t i=0; i<8000000; i++);
    
    uart_puts("[Medium] Finished.\n");
    
    while(1) task_sleep(1000);
}
void idle_task(void) 
{
  while (1) 
  {
    __asm volatile("wfi"); // Wait for interrupt until next SysTick
  }
}
void shell_task(void)
{
  char line_buffer[64];
  uint32_t idx = 0;
  uint32_t recv_val;
  
  uart_puts("\nrtos> ");
  while(1)
  {
    msg_queue_receive(&shell_rx_queue, &recv_val);
    char c = (char)recv_val;
    
    if(c == '\r' || c == '\n')
    {
      uart_puts("\n");
      line_buffer[idx] = '\0';
      
      if (line_buffer[0] == 'h' && line_buffer[1] == 'e' && line_buffer[2] == 'l' && line_buffer[3] == 'p' && line_buffer[4] == '\0')
      {
        uart_puts("Available commands: help, ps, clear, uptime, free, stats, stack\n");
      }
      else if (line_buffer[0] == 'p' && line_buffer[1] == 's' && line_buffer[2] == '\0')
      {
        scheduler_print_tasks();
      }
      else if (line_buffer[0] == 'c' && line_buffer[1] == 'l' && line_buffer[2] == 'e' && line_buffer[3] == 'a' && line_buffer[4] == 'r' && line_buffer[5] == '\0')
      {
        uart_puts("\033[2J\033[H");
      }
      else if(line_buffer[0] == 'u' && line_buffer[1] == 'p' && line_buffer[2] == 't' && line_buffer[3] == 'i' && line_buffer[4] == 'm' && line_buffer[5] == 'e' && line_buffer[6] == '\0')
      {
        uart_puts("Current time: ");
        uart_print_num(scheduler_get_ticks());
        uart_puts(" ticks\n");
      }
      else if(line_buffer[0] == 'f' && line_buffer[1] == 'r' && line_buffer[2] == 'e' && line_buffer[3] == 'e' && line_buffer[4] == '\0')
      {
        uart_puts("Free blocks: ");
        uart_print_num(pool_get_free_blocks(&my_pool));
        uart_puts("\n");
      }
      else if(line_buffer[0] == 's' && line_buffer[1] == 't' && line_buffer[2] == 'a' && line_buffer[3] == 't' && line_buffer[4] == 's' && line_buffer[5] == '\0')
      {
        uart_puts("Context Switches:");
        uart_print_num(scheduler_get_context_switches());
        uart_puts("\n");
      }
      else if(line_buffer[0] == 's' && line_buffer[1] == 't' && line_buffer[2] == 'a' && line_buffer[3] == 'c' && line_buffer[4] == 'k' && line_buffer[5] == '\0')
      {
        scheduler_print_stacks();
      }
      else if (idx > 0)
      {
        uart_puts("Unknown command\n");
      }
      
      idx = 0;
      uart_puts("rtos> ");
    }
    else if (c == '\b' || c == 0x7F)
    {
      if (idx > 0)
      {
        idx--;
        uart_puts("\b \b");
      }
    }
    else
    {
      if (idx < 63)
      {
        line_buffer[idx] = c;
        idx++;
        uart_putc(c);
      }
    }
  }
}
int main(void) 
{
  uart_init();
  msg_queue_init(&my_mq, mq_buffer, MQ_CAPACITY);
  msg_queue_init(&shell_rx_queue, shell_rx_buffer, 64);

  timer_create(&periodic_timer, 50, 1, periodic_callback);
  timer_start(&periodic_timer);
  timer_create(&oneshot_timer, 200, 0, oneshot_callback);
  timer_start(&oneshot_timer);

  pool_init(&my_pool, my_pool_buffer, BLOCK_SIZE, TOTAL_BLOCKS);

  mutex_init(&demo_mutex);
  scheduler_init();
  task_create(&high_tcb, task_high, high_stack, 10);
  task_create(&medium_tcb, task_medium, medium_stack, 20);
  task_create(&low_tcb, task_low, low_stack, 30);
  task_create(&shell_tcb, shell_task, shell_task_stack, 40);
  task_create(&idle_tcb, idle_task, idle_task_stack, 255);
  scheduler_add_task(&high_tcb);
  scheduler_add_task(&medium_tcb);
  scheduler_add_task(&low_tcb);
  scheduler_add_task(&shell_tcb);
  scheduler_add_task(&idle_tcb);
  systick_init(120000);
  scheduler_start();

  // lock CPU
  while (1) {
  }
  return 0; // never reach
}
