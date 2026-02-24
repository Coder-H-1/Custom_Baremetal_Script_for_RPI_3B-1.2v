#include "framebuffer.h"
#include "mailbox.h"
#include "font8x8.h"


framebuffer_t fb;

static int cursor_x = 0;
static int cursor_y = 0;

#define FONT_W 8
#define FONT_H 8


int fb_init(uint32_t w, uint32_t h) {
    mailbox[0] = 35 * 4;
    mailbox[1] = 0;

    mailbox[2] = 0x48003; // set physical size
    mailbox[3] = 8;
    mailbox[4] = 8;
    mailbox[5] = w;
    mailbox[6] = h;

    mailbox[7] = 0x48004; // set virtual size
    mailbox[8] = 8;
    mailbox[9] = 8;
    mailbox[10] = w;
    mailbox[11] = h;

    mailbox[12] = 0x48005; // depth
    mailbox[13] = 4;
    mailbox[14] = 4;
    mailbox[15] = 32;

    mailbox[16] = 0x40001; // allocate framebuffer
    mailbox[17] = 8;
    mailbox[18] = 8;
    mailbox[19] = 16;
    mailbox[20] = 0;

    mailbox[21] = 0x40008; // get pitch
    mailbox[22] = 4;
    mailbox[23] = 4;
    mailbox[24] = 0;

    mailbox[25] = 0;

    mailbox_call(MAILBOX_CH_PROP);

    fb.width = w;
    fb.height = h;
    fb.pitch = mailbox[24];
    fb.buffer = (uint32_t*)((uint64_t)mailbox[19] & 0x3FFFFFFF);

    return fb.buffer != 0;
}

void fb_put_pixel(int x, int y, uint32_t color) {
    fb.buffer[y * (fb.pitch / 4) + x] = color;
}


void fb_draw_char(int x, int y, char c, uint32_t color) {
    for (int row = 0; row < 8; row++) {
        uint8_t bits = font8x8[(int)c][row];
        for (int col = 0; col < 8; col++) {
            if (bits & (1 << (7 - col)))
                fb_put_pixel(x + col, y + row, color);
        }
    }
}

void fb_draw_text(int x, int y, const char *s, uint32_t color) {
    while (*s) {
        fb_draw_char(x, y, *s++, color);
        x += 8;
    }
}

static int chars_per_line(void) {
    return fb.width / FONT_W;
}

static int lines_on_screen(void) {
    return fb.height / FONT_H;
}

static void fb_scroll(void) {
    int pitch_pixels = fb.pitch / 4;

    // Move framebuffer up by one character row (8 pixels)
    for (int y = FONT_H; y < fb.height; y++) {
        for (int x = 0; x < fb.width; x++) {
            fb.buffer[(y - FONT_H) * pitch_pixels + x] =
                fb.buffer[y * pitch_pixels + x];
        }
    }

    // Clear last line
    for (int y = fb.height - FONT_H; y < fb.height; y++) {
        for (int x = 0; x < fb.width; x++) {
            fb.buffer[y * pitch_pixels + x] = 0x000000;
        }
    }

    cursor_y--;
}

void fb_putc(char c, uint32_t color) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        // No scroll, just stop at the bottom
        if ((cursor_y + 1) * FONT_H > (int)fb.height) {
            cursor_y = fb.height / FONT_H - 1;
        }
        return;
    }

    fb_draw_char(
        cursor_x * FONT_W,
        cursor_y * FONT_H,
        c,
        color
    );

    cursor_x++;

    if ((cursor_x + 1) * FONT_W > (int)fb.width) {
        cursor_x = 0;
        cursor_y++;
        if ((cursor_y + 1) * FONT_H > (int)fb.height) {
            cursor_y = fb.height / FONT_H - 1;
        }
    }
}


void fb_clear(uint32_t color) {
    int pitch_pixels = fb.pitch / 4;

    for (uint32_t y = 0; y < fb.height; y++) {
        for (uint32_t x = 0; x < fb.width; x++) {
            fb.buffer[y * pitch_pixels + x] = color;
        }
    }

    cursor_x = 0;
    cursor_y = 0;
}

void fb_print(const char *s, uint32_t color) {
    while (*s) {
        fb_putc(*s++, color);
    }
}

void fb_print_dec(uint32_t v) {
    char buf[12];
    int i = 0;

    if (v == 0) {
        fb_print("0", COLOR_GREEN);
        return;
    }

    while (v > 0) {
        buf[i++] = '0' + (v % 10);
        v /= 10;
    }

    for (int j = i - 1; j >= 0; j--) {
        char c[2] = { buf[j], 0 };
        fb_print(c, COLOR_GREEN);
    }
}