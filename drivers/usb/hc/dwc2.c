#include "dwc2.h"
#include "../../video/framebuffer.h"
#include "../../../kernel/core/utils.h"

/* ------------------------------------------------ */
/*                  INITIALIZATION                  */
/* ------------------------------------------------ */
int dwc2_init(void)
{
    /* 1/7 Waiting AHB idle */
    fb_print("[DWC2] 1/7 Waiting AHB idle...\n", COLOR_GREEN);
    GRSTCTL = GRSTCTL_AHBIDLE;
    while (!(GRSTCTL & GRSTCTL_AHBIDLE));
    fb_print("[DWC2] 1/7 AHB idle OK\n", COLOR_GREEN);

    /* 2/7 Core Reset */
    fb_print("[DWC2] 2/7 Core Reset...\n", COLOR_GREEN);
    GRSTCTL = GRSTCTL_CSFTRST;
    while (GRSTCTL & GRSTCTL_CSFTRST);
    delay_ms(50);
    fb_print("[DWC2] 2/7 Core Reset OK\n", COLOR_GREEN);

    /* 3/7 PHY Init */
    fb_print("[DWC2] 3/7 PHY Init...\n", COLOR_GREEN);
    GUSBCFG |= GUSBCFG_PHYIF;
    fb_print("[DWC2] 3/7 PHY Init OK\n", COLOR_GREEN);

    /* 4/7 Full Reset */
    fb_print("[DWC2] 4/7 Full Reset...\n", COLOR_GREEN);
    GRSTCTL = GRSTCTL_CSFTRST;
    while (GRSTCTL & GRSTCTL_CSFTRST);
    delay_ms(50);
    fb_print("[DWC2] 4/7 Full Reset OK\n", COLOR_GREEN);

    /* 5/7 Select Host Mode */
    fb_print("[DWC2] 5/7 Select Host Mode...\n", COLOR_GREEN);
    GUSBCFG |= GUSBCFG_FORCEDEVMODE;
    delay_ms(50);
    GUSBCFG = (GUSBCFG & ~GUSBCFG_FORCEDEVMODE) | GUSBCFG_FORCEHOSTMODE;
    delay_ms(50);
    fb_print("[DWC2] 5/7 Select Host Mode OK\n", COLOR_GREEN);

    /* 6/7 Power on port */
    fb_print("[DWC2] 6/7 Power on port...\n", COLOR_GREEN);
    HPRT |= HPRT_PRTPWR;
    fb_print("[DWC2] 6/7 Power on port OK\n", COLOR_GREEN);

    /* 7/7 Enable interrupts */
    fb_print("[DWC2] 7/7 Enable interrupts...\n", COLOR_GREEN);
    GAHBCFG |= GAHBCFG_GLBLINTRMSK;
    fb_print("[DWC2] 7/7 Enable interrupts OK\n", COLOR_GREEN);

    fb_print("[DWC2] Init complete, host ready\n", COLOR_GREEN);
    return 0;
}

int dwc2_port_reset(void)
{
    fb_print("[DWC2] Port reset start...\n", COLOR_GREEN);
    uint32_t val = HPRT;
    val &= ~(HPRT_W1C_MASK | HPRT_PRTENA);
    HPRT = val | HPRT_PRTRST;
    delay_ms(50); // Root port reset delay
    HPRT = val & ~HPRT_PRTRST;
    delay_ms(50);

    /* Wait for port enable */
    fb_print("[DWC2] Waiting port enable...\n", COLOR_GREEN);
    int timeout = 1000000;
    while (!(HPRT & HPRT_PRTENA)) {
        if (--timeout == 0) {
            fb_print("[DWC2] Port enable TIMEOUT\n", COLOR_RED);
            break;
        }
    }

    if (timeout > 0) {
        fb_print("[DWC2] Port enabled OK\n", COLOR_GREEN);
        delay_ms(50); // Mandatory stabilization delay
    }

    return 0;
}

/* ------------------------------------------------ */
/*                  TRANSFER LOGIC                  */
/* ------------------------------------------------ */

static int wait_for_channel(int ch)
{
    uint64_t start = get_timer();
    while (!(HCINT(ch) & (HCINT_XFERC | HCINT_CHHLTD))) {
        if (get_timer() - start > 1000000) { // 1 second timeout
            fb_print("[USB] Channel timeout\n", COLOR_RED);
            return -1;
        }
    }

    uint32_t ints = HCINT(ch);
    HCINT(ch) = 0xFFFFFFFF; // Clear all interrupts

    if (ints & HCINT_XACTERR) return -2;
    if (ints & HCINT_STALL)   return -3;
    if (ints & HCINT_NAK)     return -4;

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
    uint32_t speed = (HPRT & HPRT_PRTSPD_MASK) >> 17;
    uint32_t lsdev = (speed == 2) ? HCCHAR_LSDEV : 0;
    uint32_t mps = (speed == 2) ? 8 : 64; // Use 64 as default for Root device
    int ch = 0;

    /* 1. SETUP STAGE */
    fb_print("[USB-S]", 0xAAAAAA);
    HCINT(ch) = 0xFFFFFFFF;
    HCDMA(ch) = (uint32_t)(uintptr_t)setup;
    HCTSIZ(ch) = (8 & HCTSIZ_XFERSIZE_MASK) | (1 << 19) | HCTSIZ_PID_SETUP;
    HCCHAR(ch) = (mps << 0) | (dev_addr << 22) | lsdev | HCCHAR_EPTYPE_CTRL | HCCHAR_CHENA;

    if (wait_for_channel(ch) < 0) return -1;

    /* 2. DATA STAGE */
    if (len > 0) {
        fb_print("[USB-D]", 0xAAAAAA);
        HCINT(ch) = 0xFFFFFFFF;
        HCDMA(ch) = (uint32_t)(uintptr_t)data;
        HCTSIZ(ch) = (len & HCTSIZ_XFERSIZE_MASK) | (((len + mps - 1) / mps) << 19) | HCTSIZ_PID_DATA1;
        HCCHAR(ch) = (mps << 0) | (dev_addr << 22) | lsdev | HCCHAR_EPTYPE_CTRL | HCCHAR_CHENA | (in ? HCCHAR_EPDIR_IN : 0);

        if (wait_for_channel(ch) < 0) return -1;
    }

    /* status stage data toggle: always DATA1 */
    /* status direction: opposite of data stage */

    /* 3. STATUS STAGE */
    fb_print("[USB-Z]", 0xAAAAAA);
    HCINT(ch) = 0xFFFFFFFF;
    HCDMA(ch) = 0;
    HCTSIZ(ch) = (1 << 19) | HCTSIZ_PID_DATA1; // PID_DATA1 for status
    HCCHAR(ch) = (mps << 0) | (dev_addr << 22) | lsdev | HCCHAR_EPTYPE_CTRL | HCCHAR_CHENA | (in ? 0 : HCCHAR_EPDIR_IN);

    if (wait_for_channel(ch) < 0) return -1;

    return 0;
}

int dwc2_bulk_transfer(
    uint8_t dev_addr,
    uint8_t ep,
    uint8_t *data,
    uint32_t len,
    int in
)
{
    // ... not needed for discovery ...
    return -1;
}

int dwc2_interrupt_transfer(
    uint8_t dev_addr,
    uint8_t ep,
    uint8_t *data,
    uint32_t len,
    int in
)
{
    // ... not needed for discovery ...
    return -1;
}