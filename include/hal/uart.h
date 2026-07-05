#ifndef HAL_UART_H
#define HAL_UART_H

#include <stdint.h>

void uart_init(void);
void uart_putc(char c);
void uart_puts(const char *s);
void uart_print_num(uint32_t num);

#endif
