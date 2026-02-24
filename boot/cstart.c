/* =========================================================================
 * Project NEXUS
 * C runtime entry point (AArch64 bare-metal)
 * ========================================================================= */

#include <stdint.h>

/* Symbols provided by linker.ld */
extern uint8_t __bss_start[];
extern uint8_t __bss_end[];

/* Kernel entry */
extern void kernel_main(void);

/* -------------------------------------------------------------------------
 * C runtime start
 * ------------------------------------------------------------------------- */
void cstart(void)
{
    /* Clear BSS section */
    uint8_t *bss = __bss_start;
    while (bss < __bss_end) {
        *bss++ = 0;
    }

    /* Jump to kernel */
    kernel_main();

    /* If kernel_main returns, halt */
    while (1) {
        __asm__ volatile ("wfe");
    }
}
