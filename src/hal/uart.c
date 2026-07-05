#include "hal/uart.h"
#include "kernel/message_queue.h"
#include <stdint.h>

#define UART0_BASE      0x4000C000U
#define UART0_DR        (*(volatile uint32_t *)(UART0_BASE + 0x000))  // Data Register   
#define UART0_FR        (*(volatile uint32_t *)(UART0_BASE + 0x018))  // Flag Register   
#define UART0_IM        (*(volatile uint32_t *)(UART0_BASE + 0x038))  // Interrupt Mask Register
#define UART0_ICR       (*(volatile uint32_t *)(UART0_BASE + 0x044))  // Interrupt Clear Register
#define UART_FR_TXFF    (1U << 5)   // TX FIFO Full  

#define NVIC_EN0        (*(volatile uint32_t *)0xE000E100)  // Nested Vectored Interrupt Controller Enable Register 0

extern MessageQueue shell_rx_queue;

void uart_init(void)
{
    UART0_IM |= (1U << 4); // To enable UART RX to generate interrupt
    NVIC_EN0 |= (1U << 5); // To enable IRQ 5 (UART0) to interrupt CPU
}   

void uart_putc(char c)
{
    while (UART0_FR & UART_FR_TXFF)
    {
        // Busy waiting if full
    }
    UART0_DR = (uint32_t)c;
}

void uart_puts(const char *s)
{
    // High level function for string
    while (*s)
    {
        uart_putc(*s);
        s++;
    }
}

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

void UART0_Handler(void)
{
    char c = (char)(UART0_DR & 0xFF);
    msg_queue_send_isr(&shell_rx_queue, (uint32_t)c);
    UART0_ICR |= (1U << 4);
}