#include "dwc2.h"
#include "../../video/framebuffer.h"

static void delay(volatile int c)
{
    while (c--) asm volatile("nop");
}

/* ------------------------------------------------ */
/*                  INITIALIZATION                  */
/* ------------------------------------------------ */
void dwc2_init(void)
{
    fb_print("[USB] DWC2 Init\n", COLOR_GREEN);

    /* ------------------------------------------------ */
    /* 1️⃣ Wait for AHB idle                            */
    /* ------------------------------------------------ */
    while (!(GRSTCTL & (1 << 31)));

    /* ------------------------------------------------ */
    /* 3️⃣ Force host mode                              */
    /* ------------------------------------------------ */
    GUSBCFG |= (1 << 29);

    /* Wait until controller reports Host mode (CMOD) */
    while (!(GINTSTS & 1));   // Bit 0 = CMOD (1 = Host)

    delay(200000);

    /* 4️⃣ CRITICAL: Reset core AGAIN after mode switch */
    GRSTCTL |= 1;
    while (GRSTCTL & 1);
    delay(100000);

    /* ADD THIS RIGHT HERE */
    HCFG &= ~3;
    HCFG |= 1;   // 48 MHz FS clock
    delay(100000);

    /* ------------------------------------------------ */
    /* 5️⃣ Enable DMA + Global Interrupt                */
    /* ------------------------------------------------ */
    GAHBCFG |= (1 << 5) | (1 << 0);


    /* Enable FS/LS support */
    HCFG |= (1 << 2);   // FSLSSUPP

    /* ------------------------------------------------ */
    /* 7️⃣ Flush TX FIFO                               */
    /* ------------------------------------------------ */
    GRSTCTL |= (1 << 5) | (0x10 << 6);
    while (GRSTCTL & (1 << 5));

    /* ------------------------------------------------ */
    /* 8️⃣ Flush RX FIFO                               */
    /* ------------------------------------------------ */
    GRSTCTL |= (1 << 4);
    while (GRSTCTL & (1 << 4));

    delay(200000);

    /* ------------------------------------------------ */
    /* 9️⃣ Power the host port                         */
    /* ------------------------------------------------ */
    uint32_t hprt = HPRT;
    HPRT = hprt | (1 << 12);   // PRTPOWER  

    delay(300000);

    fb_print("[USB] Host Ready\n", COLOR_GREEN);
    fb_print("\n", COLOR_GREEN);

    fb_print("HCFG = ", COLOR_GREEN);
    fb_print_hex32(HCFG);
    fb_print("\n", COLOR_GREEN);

    fb_print("HPRT before reset = ", COLOR_GREEN);
    fb_print_hex32(HPRT);
    fb_print("\n", COLOR_GREEN);

    fb_print("HFNUM 1 = ", COLOR_GREEN);
    fb_print_hex32(HFNUM);
    fb_print("\n", COLOR_GREEN);

    delay(1000000);

    fb_print("HFNUM 2 = ", COLOR_GREEN);
    fb_print_hex32(HFNUM);
    fb_print("\n", COLOR_GREEN);

}

/* ------------------------------------------------ */
/*                   PORT RESET                    */
/* ------------------------------------------------ */

int dwc2_port_reset(void)
{
    fb_print("[USB] Port Reset\n", COLOR_GREEN);

    uint32_t hprt;

    // Ensure power on
    hprt = HPRT;
    if (!(hprt & (1 << 12))) {
        HPRT = hprt | (1 << 12);
        delay(300000);
    }

    // --- Start Reset ---
    hprt = HPRT;
    hprt |= (1 << 8);        // PRTRESET
    HPRT = hprt;

    delay(600000);           // >=50ms

    // --- Stop Reset ---
    hprt = HPRT;
    hprt &= ~(1 << 8);
    HPRT = hprt;

    // Wait until reset bit actually clears
    while (HPRT & (1 << 8));

    // Wait for port enable
    int timeout = 1000000;
    while (!(HPRT & (1 << 2))) {
        if (--timeout == 0) {
            fb_print("[USB] Port enable timeout\n", COLOR_RED);
            break;
        }
    }

    // Now clear change bits (after enable attempt)
    hprt = HPRT;
    hprt |= (1 << 1);   // PRTCONNDET
    hprt |= (1 << 3);   // PRTENCHNG
    hprt |= (1 << 5);   // PRTOVRCURRCHNG
    HPRT = hprt;

    fb_print("[USB] HFNUM 1 = ", COLOR_GREEN );
    fb_print_hex32(HFNUM);
    fb_print("\n", COLOR_GREEN);

    delay(1000000);

    fb_print("[USB] HFNUM 2 = ", COLOR_GREEN);
    fb_print_hex32(HFNUM);
    fb_print("\n", COLOR_GREEN);

    fb_print("[USB] HPRT ", COLOR_GREEN);
    fb_print_hex32(HPRT);
    fb_print("\n", COLOR_GREEN);

    return 0;
}

/* ------------------------------------------------ */
/*                CONTROL TRANSFER                 */
/* ------------------------------------------------ */

#define HCINT_XFRC   (1 << 1)

static int wait_for_xfer_complete(void)
{
    int timeout = 1000000;

    while (!(HCINT0 & HCINT_XFRC)) {
        if (--timeout == 0) {
            fb_print("[USB] Transfer timeout\n", COLOR_RED);
            return -1;
        }
    }

    uint32_t status = HCINT0;

    fb_print("[USB] HCINT0 = ", COLOR_GREEN);
    fb_print_hex((status >> 24) & 0xFF);
    fb_print_hex((status >> 16) & 0xFF);
    fb_print_hex((status >> 8) & 0xFF);
    fb_print_hex(status & 0xFF);
    fb_print("\n", COLOR_GREEN);

    HCINT0 = 0xFFFFFFFF;
    return 0;
}

int dwc2_control_transfer(
    uint8_t dev_addr,
    uint8_t *setup,
    uint8_t *data,
    uint16_t len,
    int in
)
{
    uint32_t hcchar;
    uint32_t hctsiz;

    uint32_t speed = (HPRT >> 17) & 3;
    uint32_t lsdev = (speed == 2) ? (1 << 17) : 0;    

    /* -------------------- */
    /* 1️⃣ SETUP STAGE */
    /* -------------------- */

    HCINT0 = 0xFFFFFFFF;

    // Ensure channel disabled
    HCCHAR0 |= (1 << 30);
    HCCHAR0 &= ~(1 << 31);
    while (HCCHAR0 & (1 << 31));

    HCDMA0 = (uint32_t)setup;

    hctsiz =
        (8 & 0x7FFFF) |
        (1 << 19)     |
        (3 << 29);    // PID = SETUP

    HCTSIZ0 = hctsiz;

    hcchar =
        (8  << 0)        |   // MPS
        (0  << 11)       |   // EP0
        (0  << 15)       |   // OUT
        (0  << 18)       |   // Control
        lsdev            |   // speed
        (dev_addr << 22) |
        (1  << 31);          // Enable

    HCCHAR0 = hcchar;

    if (wait_for_xfer_complete() < 0)
        return -1;

    if (!(HPRT & (1 << 2))) {
        fb_print("Port still not enabled\n", COLOR_RED);
        return -1;
    }

    /* -------------------- */
    /* 2️⃣ DATA STAGE */
    /* -------------------- */

    if (len > 0)
    {
        HCINT0 = 0xFFFFFFFF;

        HCCHAR0 |= (1 << 30);
        HCCHAR0 &= ~(1 << 31);
        while (HCCHAR0 & (1 << 31));

        HCDMA0 = (uint32_t)data;

        hctsiz =
            (len & 0x7FFFF) |
            (((len + 7) / 8) << 19) |
            (1 << 29);       // DATA1

        HCTSIZ0 = hctsiz;

        hcchar =
            (8  << 0) |
            (0  << 11) |
            ((in ? 1 : 0) << 15) |
            (0  << 18) |
            lsdev      |
            (dev_addr << 22) |
            (1  << 31);

        HCCHAR0 = hcchar;

        if (wait_for_xfer_complete() < 0)
            return -1;
    }

    /* -------------------- */
    /* 3️⃣ STATUS STAGE */
    /* -------------------- */

    HCINT0 = 0xFFFFFFFF;

    HCCHAR0 |= (1 << 30);
    HCCHAR0 &= ~(1 << 31);
    while (HCCHAR0 & (1 << 31));

    hctsiz =
        (0 << 0) |
        (1 << 19) |
        (1 << 29);   // DATA1

    HCTSIZ0 = hctsiz;

    hcchar =
        (8  << 0) |
        (0  << 11) |
        ((in ? 0 : 1) << 15) |
        (0  << 18) |
        lsdev |
        (dev_addr << 22) |
        (1  << 31);

    HCCHAR0 = hcchar;

    if (wait_for_xfer_complete() < 0)
        return -1;

    return 0;
}