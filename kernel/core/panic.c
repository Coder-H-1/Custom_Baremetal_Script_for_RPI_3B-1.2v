#include "panic.h"
#include "../../drivers/video/framebuffer.h"
#include "utils.h"

void panic(const char *msg) {
    fb_print("\n!!! KERNEL PANIC !!!\n", COLOR_RED);
    fb_print(msg, COLOR_RED);
    fb_print("\n", COLOR_RED);
    halt();
}
