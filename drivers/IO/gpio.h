// #ifndef GPIO_H
// #define GPIO_H

// #include <stdint.h>

// // GPIO function select
// #define GPIO_INPUT  0
// #define GPIO_OUTPUT 1
// #define GPIO_ALT0   4
// #define GPIO_ALT1   5
// #define GPIO_ALT2   6
// #define GPIO_ALT3   7
// #define GPIO_ALT4   3
// #define GPIO_ALT5   2

// void gpio_set_function(uint8_t pin, uint8_t func);
// void gpio_write(uint8_t pin, int value);
// int  gpio_read(uint8_t pin);
// void delay_ms(float ms);

// #endif

#pragma once
#include <stdint.h>

// GPIO Base
#define GPIO_BASE 0x3F200000UL

// Registers
#define GPFSEL0  (*(volatile uint32_t*)(GPIO_BASE + 0x00))
#define GPFSEL1  (*(volatile uint32_t*)(GPIO_BASE + 0x04))
#define GPFSEL2  (*(volatile uint32_t*)(GPIO_BASE + 0x08))
#define GPFSEL3  (*(volatile uint32_t*)(GPIO_BASE + 0x0C))
#define GPFSEL4  (*(volatile uint32_t*)(GPIO_BASE + 0x10))
#define GPFSEL5  (*(volatile uint32_t*)(GPIO_BASE + 0x14))

#define GPSET0   (*(volatile uint32_t*)(GPIO_BASE + 0x1C))
#define GPSET1   (*(volatile uint32_t*)(GPIO_BASE + 0x20))

#define GPCLR0   (*(volatile uint32_t*)(GPIO_BASE + 0x28))
#define GPCLR1   (*(volatile uint32_t*)(GPIO_BASE + 0x2C))

#define GPLEV0   (*(volatile uint32_t*)(GPIO_BASE + 0x34))
#define GPLEV1   (*(volatile uint32_t*)(GPIO_BASE + 0x38))

#define GPPUD    (*(volatile uint32_t*)(GPIO_BASE + 0x4C))
#define GPPUDCLK0 (*(volatile uint32_t*)(GPIO_BASE + 0x50))
#define GPPUDCLK1 (*(volatile uint32_t*)(GPIO_BASE + 0x54))

// GPIO Modes
typedef enum {
    GPIO_INPUT  = 0,
    GPIO_OUTPUT = 1,
    GPIO_ALT0   = 4,
    GPIO_ALT1   = 5,
    GPIO_ALT2   = 6,
    GPIO_ALT3   = 7,
    GPIO_ALT4   = 3,
    GPIO_ALT5   = 2
} gpio_mode_t;

// API
void gpio_set_mode(uint8_t pin, gpio_mode_t mode);
void gpio_write(uint8_t pin, uint8_t value);
uint8_t gpio_read(uint8_t pin);
void gpio_set_pull(uint8_t pin, uint8_t pud);

