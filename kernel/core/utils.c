#include "utils.h"
#include "../../drivers/video/framebuffer.h"

/* ------------ delays ------------ */

/* ------------ delays ------------ */

#define SYSTIMER_CLO (*(volatile uint32_t*)0x3F003004)
#define SYSTIMER_CHI (*(volatile uint32_t*)0x3F003008)

uint64_t get_timer(void) {
    uint32_t hi = SYSTIMER_CHI;
    uint32_t lo = SYSTIMER_CLO;
    if (hi != SYSTIMER_CHI) {
        hi = SYSTIMER_CHI;
        lo = SYSTIMER_CLO;
    }
    return ((uint64_t)hi << 32) | lo;
}

void delay_cycles(uint32_t c) {
    while (c--) asm volatile("nop");
}

void delay_us(uint32_t us) {
    uint64_t start = get_timer();
    while (get_timer() - start < us);
}

void delay_ms(uint32_t ms) {
    delay_us(ms * 1000);
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

int strcmp(const char *a, const char *b) {
    while (*a && (*a == *b)) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

int strncmp(const char *a, const char *b, size_t n) {
    while (n-- && *a && (*a == *b)) { a++; b++; }
    if (n == (size_t)-1) return 0;
    return (unsigned char)*a - (unsigned char)*b;
}

char *strcpy(char *dst, const char *src) {
    char *d = dst;
    while ((*d++ = *src++));
    return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
    char *d = dst;
    while (n && (*d++ = *src++)) n--;
    while (n--) *d++ = '\0';
    return dst;
}

/* Reentrant string tokenizer */
char *strtok_r(char *s, const char *delim, char **saveptr) {
    if (s == 0) s = *saveptr;
    /* skip leading delimiters */
    while (*s) {
        const char *d = delim;
        int is_delim = 0;
        while (*d) { if (*s == *d++) { is_delim = 1; break; } }
        if (!is_delim) break;
        s++;
    }
    if (*s == '\0') { *saveptr = s; return 0; }
    char *token = s;
    while (*s) {
        const char *d = delim;
        while (*d) {
            if (*s == *d) { *s = '\0'; *saveptr = s + 1; return token; }
            d++;
        }
        s++;
    }
    *saveptr = s;
    return token;
}

/* Integer to ASCII: base 10 or 16 */
void itoa(uint32_t v, char *buf, int base) {
    const char digits[] = "0123456789ABCDEF";
    char tmp[11];
    int i = 0;
    if (v == 0) { buf[0] = '0'; buf[1] = 0; return; }
    while (v && i < 10) { tmp[i++] = digits[v % base]; v /= base; }
    int j = 0;
    while (i > 0) buf[j++] = tmp[--i];
    buf[j] = 0;
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
