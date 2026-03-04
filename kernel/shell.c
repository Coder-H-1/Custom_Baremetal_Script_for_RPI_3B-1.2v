/* =========================================================================
 * Project NEXUS — Minimal Shell
 * Focus: Status Display and UART Fallback
 * ========================================================================= */

#include "shell.h"
#include "core/utils.h"
#include "../drivers/video/framebuffer.h"
#include "../drivers/uart/uart.h"

static char  input_buf[SHELL_BUF_SIZE];
static int   input_len;
static volatile uint32_t shell_ticks;

extern uint8_t __bss_start[];
extern uint8_t __bss_end[];
extern uint8_t __kernel_end[];

void shell_puts(const char *s) {
    uart_puts(s);
    fb_print(s, COLOR_WHITE);
}

static void shell_puts_c(const char *s, uint32_t color) {
    uart_puts(s);
    fb_print(s, color);
}

static void shell_putc(char c) {
    uart_putc(c);
    fb_putc(c, COLOR_WHITE);
}

void shell_init(void) {
    fb_clear(0x000000);
    shell_puts_c("\n==================================\n", 0x00BFFF);
    shell_puts_c("  Project NEXUS  v0.1  (USB-LOG)  \n", 0x00BFFF);
    shell_puts_c("==================================\n", 0x00BFFF);
    shell_puts_c("USB Discovery focused build.\n", COLOR_WHITE);
    shell_puts_c("Input: UART (serial console)\n\n", 0xFFAA00);
    
    input_len = 0;
    input_buf[0] = '\0';
    shell_puts_c("nexus> ", COLOR_GREEN);
}

void shell_process_char(char c) {
    shell_ticks++;
    if (c == '\r' || c == '\n') {
        shell_putc('\n');
        shell_puts_c("nexus> ", COLOR_GREEN);
        input_len = 0;
    } else if (input_len < SHELL_BUF_SIZE - 1) {
        input_buf[input_len++] = c;
        shell_putc(c);
    }
}

void shell_run(void) {
    /* Basic non-blocking UART read */
    if (!(*(volatile uint32_t*)(0x3F201018) & (1 << 4))) {
        char c = (char)(*(volatile uint32_t*)(0x3F201000) & 0xFF);
        shell_process_char(c);
    }
}
