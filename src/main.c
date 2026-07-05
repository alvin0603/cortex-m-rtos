// Entry point for Mini RTOS

#include "hal/systick.h"
#include "hal/uart.h"
#include "kernel/message_queue.h"
#include "kernel/scheduler.h"
#include "kernel/software_timer.h"
#include "kernel/task.h"
#include "kernel/memory_pool.h"


uint32_t task_1_stack[TASK_STACK_SIZE];
uint32_t task_2_stack[TASK_STACK_SIZE];
uint32_t idle_task_stack[TASK_STACK_SIZE];
TCB tcb1, tcb2;
TCB idle_tcb;

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

void task1(void) 
{
  uint32_t count = 0;
  while (1) 
  {
    SensorData *sensor_ptr = (SensorData *)pool_alloc(&my_pool);
    if (sensor_ptr != NULL) 
    {
      sensor_ptr->id = 1;
      sensor_ptr->data = count;
      // print_timestamp();
      // uart_puts("Producer allocated ptr and sent\n");
      msg_queue_send(&my_mq, (uint32_t)sensor_ptr);
      count++;
    } 
    else 
    {
      // print_timestamp();
      // uart_puts("Producer OOM!\n");
    }
    task_sleep(20);
  }
}
void task2(void) 
{
  uint32_t recv_val;
  while (1) 
  {
    msg_queue_receive(&my_mq, &recv_val);
    SensorData *sensor_ptr = (SensorData *)recv_val;
    // print_timestamp();
    // uart_puts("  Consumer read data: ");
    // uart_print_num(sensor_ptr->data);
    // uart_puts("\n");
    pool_free(&my_pool, sensor_ptr);
    // uart_puts("  Consumer freed ptr\n");
    task_sleep(50);
  }
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
        uart_puts("Available commands: help, ps, clear, uptime, free\n");
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

  scheduler_init();
  task_create(&tcb1, task1, task_1_stack, 0);
  task_create(&tcb2, task2, task_2_stack, 1);
  task_create(&shell_tcb, shell_task, shell_task_stack, 2);
  task_create(&idle_tcb, idle_task, idle_task_stack, 255);
  scheduler_add_task(&tcb1);
  scheduler_add_task(&tcb2);
  scheduler_add_task(&shell_tcb);
  scheduler_add_task(&idle_tcb);
  systick_init(120000);
  scheduler_start();

  // lock CPU
  while (1) {
  }
  return 0; // never reach
}
