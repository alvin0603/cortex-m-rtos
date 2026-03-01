// Entry point for Mini RTOS

#include "hal/uart.h"
int main(void)
{
    uart_init();

    uart_puts("\n");
    uart_puts("Hello from Mini RTOS!\n");
    uart_puts("UART output is working.\n");
    uart_puts("\n");
    uart_puts("System halted.\n");

    // lock CPU
    while (1){}
    return 0; // never reach
}
