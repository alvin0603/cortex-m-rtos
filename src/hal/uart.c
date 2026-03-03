#include "hal/uart.h"
#include <stdint.h>

#define UART0_BASE      0x4000C000U
#define UART0_DR        (*(volatile uint32_t *)(UART0_BASE + 0x000))  // Data Register   
#define UART0_FR        (*(volatile uint32_t *)(UART0_BASE + 0x018))  // Flag Register   
#define UART_FR_TXFF    (1U << 5)   // TX FIFO Full  

void uart_init(void)
{
    // For the future setting to real hardware (STM32)
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
