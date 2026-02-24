#include "mailbox.h"

volatile uint32_t mailbox[36] __attribute__((aligned(16)));

void mailbox_call(uint8_t channel) {
    uint32_t r = ((uint32_t)((uintptr_t)mailbox) & ~0xF) | (channel & 0xF);

    while (*MAILBOX_STATUS & MAILBOX_FULL);
    *MAILBOX_WRITE = r;

    while (1) {
        while (*MAILBOX_STATUS & MAILBOX_EMPTY);
        if (*MAILBOX_READ == r)
            return;
    }
}
