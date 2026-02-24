#pragma once
#include <stdint.h>

// Base address
#define UART0_BASE 0x3F201000UL

// Registers
#define UART0_DR      (*(volatile uint32_t*)(UART0_BASE + 0x00))
#define UART0_FR      (*(volatile uint32_t*)(UART0_BASE + 0x18))
#define UART0_IBRD    (*(volatile uint32_t*)(UART0_BASE + 0x24))
#define UART0_FBRD    (*(volatile uint32_t*)(UART0_BASE + 0x28))
#define UART0_LCRH    (*(volatile uint32_t*)(UART0_BASE + 0x2C))
#define UART0_CR      (*(volatile uint32_t*)(UART0_BASE + 0x30))
#define UART0_IMSC    (*(volatile uint32_t*)(UART0_BASE + 0x38))
#define UART0_ICR     (*(volatile uint32_t*)(UART0_BASE + 0x44))

// Functions
void uart_init(void);
void uart_putc(char c);
char uart_getc(void);
void uart_puts(const char *s);
