#ifndef SHELL_H
#define SHELL_H

#include <stdint.h>

#define SHELL_BUF_SIZE 128

void shell_init(void);
void shell_run(void);
void shell_process_char(char c);

#endif
