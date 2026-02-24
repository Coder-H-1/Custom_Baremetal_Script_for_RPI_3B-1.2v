#ifndef MAILBOX_H
#define MAILBOX_H

#include <stdint.h>

#define MAILBOX_BASE   0x3F00B880
#define MAILBOX_READ   ((volatile uint32_t*)(MAILBOX_BASE + 0x00))
#define MAILBOX_STATUS ((volatile uint32_t*)(MAILBOX_BASE + 0x18))
#define MAILBOX_WRITE  ((volatile uint32_t*)(MAILBOX_BASE + 0x20))

#define MAILBOX_EMPTY  0x40000000
#define MAILBOX_FULL   0x80000000
#define MAILBOX_CH_PROP 8

void mailbox_call(uint8_t channel);

extern volatile uint32_t mailbox[36];

#endif
