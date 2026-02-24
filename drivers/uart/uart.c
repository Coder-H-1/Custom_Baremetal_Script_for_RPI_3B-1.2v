#include "uart.h"
#include "../IO/gpio.h"
#include "../../kernel/core/utils.h"

// Simple delay for UART setup
static void uart_delay(int32_t cnt) {
    while (cnt--) asm volatile("nop");
}

// GPIO pins for UART0 (ALT0)
#define UART0_TX_PIN 14
#define UART0_RX_PIN 15

void uart_init(void) {
    // Disable UART0
    UART0_CR = 0;

    // Configure GPIO pins
    gpio_set_mode(UART0_TX_PIN, GPIO_ALT0);
    gpio_set_mode(UART0_RX_PIN, GPIO_ALT0);
    gpio_set_pull(UART0_TX_PIN, 0);
    gpio_set_pull(UART0_RX_PIN, 0);

    uart_delay(150);

    // Clear pending interrupts
    UART0_ICR = 0x7FF;

    // Baud rate 115200
    // Assumes 48 MHz UART clock (Pi default)
    // IBRD = int(48_000_000 / (16 * 115200)) = 26
    // FBRD = int(0.730 * 64 + 0.5) = 47
    UART0_IBRD = 26;
    UART0_FBRD = 3; // sometimes 3 or 47 depending on UART clk; adjust if needed

    // Line control: 8 bits, no parity, 1 stop, FIFO enabled
    UART0_LCRH = (3 << 5) | (1 << 4);

    // Enable UART, TX/RX
    UART0_CR = (1 << 0) | (1 << 8) | (1 << 9);
}

void uart_putc(char c) {
    // Wait until TX not full
    while (UART0_FR & (1 << 5));
    UART0_DR = c;
}

char uart_getc(void) {
    // Wait until RX not empty
    while (UART0_FR & (1 << 4));
    return UART0_DR & 0xFF;
}

void uart_puts(const char *s) {
    while (*s) {
        if (*s == '\n') uart_putc('\r'); // convert LF to CRLF
        uart_putc(*s++);
    }
}
