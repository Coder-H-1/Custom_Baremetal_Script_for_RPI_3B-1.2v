#include "shell.h"
#include "../drivers/video/framebuffer.h"

static char input_buf[SHELL_BUF_SIZE];
static int input_len = 0;

void shell_init(void) {
    fb_clear(0x000000);          // clear screen
    fb_print("Nexus Baremetal Shell\n\n", 0xFFFFFF);
    fb_print("\nnexus> ", 0x00FF00);
    input_len = 0;
}

// This will be called whenever a key is pressed
void shell_process_char(char c) {
    if (c == '\r' || c == '\n') {
        fb_print("\n", 0xFFFFFF);

        // Simple command echo for now
        fb_print("You typed: ", 0xFFFF00);
        fb_print(input_buf, 0xFFFFFF);
        fb_print("\n", 0xFFFFFF);

        // Reset input
        input_len = 0;
        input_buf[0] = 0;

        // Print prompt again
        fb_print("\nnexus> ", 0x00FF00);
        return;
    }

    if (input_len < SHELL_BUF_SIZE - 1) {
        input_buf[input_len++] = c;
        input_buf[input_len] = 0;
        fb_putc(c, 0xFFFFFF); // echo typed character
    }
}

void shell_run(void) {
    // For now, this is empty.
    // Later we’ll poll keyboard or use IRQs to call shell_process_char()
}
