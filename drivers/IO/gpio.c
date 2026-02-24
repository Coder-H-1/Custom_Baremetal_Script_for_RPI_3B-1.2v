// #include "gpio.h"

// #define GPIO_BASE 0x3F200000UL

// volatile uint32_t* GPFSEL0 = (uint32_t*)(GPIO_BASE + 0x00);
// volatile uint32_t* GPSET0  = (uint32_t*)(GPIO_BASE + 0x1C);
// volatile uint32_t* GPCLR0  = (uint32_t*)(GPIO_BASE + 0x28);
// volatile uint32_t* GPLEV0  = (uint32_t*)(GPIO_BASE + 0x34);

// // Set GPIO pin function (input/output/alt)
// void gpio_set_function(uint8_t pin, uint8_t func) {
//     volatile uint32_t* reg = GPFSEL0 + (pin / 10);
//     int shift = (pin % 10) * 3;
//     *reg &= ~(7 << shift);     // Clear the 3 bits
//     *reg |= ((func & 7) << shift); // Set new function
// }

// // Write GPIO pin (0 or 1)
// void gpio_write(uint8_t pin, int value) {
//     if (value)
//         *(GPSET0 + (pin / 32)) = 1 << (pin % 32);
//     else
//         *(GPCLR0 + (pin / 32)) = 1 << (pin % 32);
// }

// // Read GPIO pin
// int gpio_read(uint8_t pin) {
//     return (*(GPLEV0 + (pin / 32)) >> (pin % 32)) & 1;
// }

// // Simple busy-loop delay
// void delay_ms(float ms) {
//     for (volatile unsigned long i = 0; i < (ms * 500000); i++);
// }


#include "gpio.h"
#include "../video/framebuffer.h"

// Simple delay (for pull-up/down timing)
static void gpio_delay(int32_t cnt) {
    while (cnt--) asm volatile("nop");
}

void gpio_set_mode(uint8_t pin, gpio_mode_t mode) {
    uint32_t reg = pin / 10;
    uint32_t shift = (pin % 10) * 3;
    volatile uint32_t *fsel = &GPFSEL0 + reg;

    uint32_t val = *fsel;
    val &= ~(0b111 << shift); // Clear
    val |= ((mode & 0b111) << shift); // Set new mode
    *fsel = val;
}

void gpio_write(uint8_t pin, uint8_t value) {
    if (value)
        (pin < 32) ? (GPSET0 = 1 << pin) : (GPSET1 = 1 << (pin - 32));
    else
        (pin < 32) ? (GPCLR0 = 1 << pin) : (GPCLR1 = 1 << (pin - 32));
}

uint8_t gpio_read(uint8_t pin) {
    return (pin < 32) ? ((GPLEV0 >> pin) & 1) : ((GPLEV1 >> (pin - 32)) & 1);
}

void gpio_set_pull(uint8_t pin, uint8_t pud) {
    GPPUD = pud & 0b11;
    gpio_delay(150);
    if (pin < 32)
        GPPUDCLK0 = 1 << pin;
    else
        GPPUDCLK1 = 1 << (pin - 32);
    gpio_delay(150);
    GPPUD = 0;
    GPPUDCLK0 = 0;
    GPPUDCLK1 = 0;
}
