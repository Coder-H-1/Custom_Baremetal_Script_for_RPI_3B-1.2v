#ifndef SHELL_H
#define SHELL_H

#include <stdint.h>

#define SHELL_BUF_SIZE  256
#define SHELL_MAX_ARGS  16

void shell_init(void);
void shell_run(void);
void shell_process_char(char c);
void shell_puts(const char *s);   /* mirror output to UART + framebuffer */

#endif
