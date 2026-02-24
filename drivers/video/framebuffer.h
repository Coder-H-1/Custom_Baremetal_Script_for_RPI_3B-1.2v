#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#define COLOR_WHITE 0xFFFFFF
#define COLOR_BLACK 0x000000
#define COLOR_RED   0xFF0000
#define COLOR_GREEN 0x00FF00
#define COLOR_BLUE  0x0000FF
#define COLOR_TERM  0x808080
#include <stdint.h>

void fb_putc(char c, uint32_t color);
void fb_print(const char *s, uint32_t color);

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t *buffer;
} framebuffer_t;

int fb_init(uint32_t w, uint32_t h);
void fb_put_pixel(int x, int y, uint32_t color);
void fb_draw_char(int x, int y, char c, uint32_t color);
void fb_draw_text(int x, int y, const char *s, uint32_t color);
void fb_clear(uint32_t color);

extern framebuffer_t fb;

// Print a single byte as 2-digit hex
static void fb_print_hex(uint8_t val) {
    char hex[3];
    hex[0] = "0123456789ABCDEF"[val >> 4];
    hex[1] = "0123456789ABCDEF"[val & 0xF];
    hex[2] = 0;
    fb_print(hex, 0x00FF00);
}

// Optional: print 32-bit value as 8-digit hex
static void fb_print_hex32(uint32_t val) {
    char hex[9];
    for (int i = 0; i < 8; i++) {
        hex[7-i] = "0123456789ABCDEF"[ (val >> (i*4)) & 0xF ];
    }
    hex[8] = 0;
    fb_print(hex, 0x00FF00);
}

void fb_print_dec(uint32_t v);

#endif
