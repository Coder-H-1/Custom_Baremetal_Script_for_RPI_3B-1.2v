#include "utils.h"
#include "../../drivers/video/framebuffer.h"

/* ------------ delays ------------ */

void delay_cycles(uint32_t c) {
    while (c--){
        asm volatile("nop");
    }
}

void delay_us(uint32_t us) {
    while (us--)
        delay_cycles(6);
}

void delay_ms(uint32_t ms) {
    while (ms--)
        delay_us(1000);
}

/* ------------ memory ------------ */

void *memcpy(void *d, const void *s, size_t n) {
    uint8_t *dst = d;
    const uint8_t *src = s;
    while (n--) *dst++ = *src++;
    return d;
}

void *memset(void *d, int v, size_t n) {
    uint8_t *dst = d;
    while (n--) *dst++ = (uint8_t)v;
    return d;
}

int memcmp(const void *a, const void *b, size_t n) {
    const uint8_t *x = a, *y = b;
    while (n--) {
        if (*x != *y)
            return *x - *y;
        x++; y++;
    }
    return 0;
}

/* ------------ string ------------ */

size_t strlen(const char *s) {
    size_t n = 0;
    while (*s++) n++;
    return n;
}

/* ------------ printing ------------ */

static const char hex[] = "0123456789ABCDEF";

void print_hex8(uint8_t v) {
    char s[3];
    s[0] = hex[(v >> 4) & 0xF];
    s[1] = hex[v & 0xF];
    s[2] = 0;
    fb_print(s, COLOR_GREEN);
}

void print_hex16(uint16_t v) {
    print_hex8(v >> 8);
    print_hex8(v & 0xFF);
}

void print_hex32(uint32_t v) {
    print_hex16(v >> 16);
    print_hex16(v & 0xFFFF);
}

void print_dec(uint32_t v) {
    char buf[11];
    int i = 10;
    buf[i--] = 0;

    if (v == 0) {
        fb_print("0", COLOR_GREEN);
        return;
    }

    while (v && i >= 0) {
        buf[i--] = '0' + (v % 10);
        v /= 10;
    }
    fb_print(&buf[i + 1], COLOR_GREEN);
}

/* ------------ misc ------------ */

void halt(void) {
    fb_print("\n[HALT]\n", COLOR_RED);
    while (1)
        asm volatile("wfe");
}


// Xorshift32 state
static uint32_t state = 2463534242; // default seed

// Seed the generator
void srand(uint32_t seed) {
    if (seed != 0) state = seed;
}

// Return pseudo-random 32-bit number
uint32_t rand32(void) {
    uint32_t x = state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    state = x;
    return x;
}

// Return pseudo-random number in [min, max)
uint32_t rand_range(uint32_t min, uint32_t max) {
    if (max <= min) return min;
    uint32_t r = rand32();
    return min + (r % (max - min));
}
