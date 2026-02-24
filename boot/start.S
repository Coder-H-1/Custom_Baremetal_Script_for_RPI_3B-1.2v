/* =========================================================================
 * Project NEXUS
 * AArch64 bare-metal boot code for Raspberry Pi 3
 * ========================================================================= */

.section .text
.global _start
.extern cstart

/* -------------------------------------------------------------------------
 * Entry point
 * ------------------------------------------------------------------------- */
_start:
    /* Disable interrupts (IRQ, FIQ, SError, Debug) */
    msr daifset, #0xf

    /* Set up stack pointer
     * Stack top symbol comes from linker.ld
     */
    ldr x0, =_stack_top
    mov sp, x0

    /* Jump to C runtime start */
    bl cstart

/* If cstart returns, hang */
1:
    wfe
    b 1b
