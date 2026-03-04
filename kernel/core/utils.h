#pragma once
#include <stdint.h>
#include <stddef.h>

#ifndef UTILS_H
#define UTILS_H


void srand(uint32_t seed);
uint32_t rand32(void);
uint32_t rand_range(uint32_t min, uint32_t max);

#endif


/* -------- delays -------- */
uint64_t get_timer(void);
void delay_cycles(uint32_t cycles);
void delay_us(uint32_t us);
void delay_ms(uint32_t ms);

/* -------- memory -------- */
void *memcpy(void *dst, const void *src, size_t n);
void *memset(void *dst, int v, size_t n);
int memcmp(const void *a, const void *b, size_t n);

/* -------- string -------- */
size_t strlen(const char *s);
int    strcmp(const char *a, const char *b);
int    strncmp(const char *a, const char *b, size_t n);
char  *strcpy(char *dst, const char *src);
char  *strncpy(char *dst, const char *src, size_t n);
char  *strtok_r(char *s, const char *delim, char **saveptr);
void   itoa(uint32_t v, char *buf, int base);

/* -------- debug printing -------- */
void print_hex8(uint8_t v);
void print_hex16(uint16_t v);
void print_hex32(uint32_t v);
void print_dec(uint32_t v);

/* -------- misc -------- */
void halt(void);
