#include "dwc2.h"
#include "../../video/framebuffer.h"

static void delay(volatile int c) {
    while (c--) asm volatile("nop");
}

void dwc2_init(void) {
    fb_print("[USB] DWC2 init\n", COLOR_GREEN);

    GRSTCTL |= 1;
    while (GRSTCTL & 1);

    GUSBCFG |= (1 << 29);  // force host
    delay(100000);

    GAHBCFG |= 1;          // global int
    HCFG = 1;              // 48MHz

    HPRT |= (1 << 12);     // power port
    delay(200000);

    fb_print("[USB] Host ready\n", COLOR_GREEN);
}

int dwc2_control_get_descriptor(uint8_t *buf) {
    // SETUP packet
    static uint8_t setup[8] = {
        0x80, // IN | Standard | Device
        0x06, // GET_DESCRIPTOR
        0x00, // Descriptor index
        0x01, // DEVICE descriptor
        0x00, 0x00, // wIndex
        0x12, 0x00  // length = 18
    };

    // Clear interrupts
    HCINT0 = 0xFFFFFFFF;
    HCINTMSK0 = 0xFFFFFFFF;

    // SETUP stage
    HCDMA0 = (uint32_t)setup;
    HCTSIZ0 = (1 << 29) | (1 << 19) | 8; // SETUP, 1 pkt, 8 bytes

    HCCHAR0 =
        (0 << 22) |    // device addr 0
        (0 << 15) |    // EP0
        (0 << 11) |    // control
        (1 << 31);     // enable

    while (!(HCINT0 & 0x02)); // XFRC

    // DATA stage (IN)
    HCINT0 = 0xFFFFFFFF;
    HCDMA0 = (uint32_t)buf;
    HCTSIZ0 = (1 << 19) | 18; // IN, 18 bytes

    HCCHAR0 =
        (0 << 22) |
        (0 << 15) |
        (0 << 11) |
        (1 << 15) |  // IN
        (1 << 31);

    while (!(HCINT0 & 0x02)); // XFRC

    return 0;
}

int dwc2_port_reset(void) {
    uint32_t hprt;

    fb_print("[USB] Root port reset start\n", COLOR_GREEN);

    // Read HPRT once
    hprt = HPRT;

    // Clear W1C bits: POCI, PEC
    hprt &= ~((1 << 2) | (1 << 5));

    // Ensure port power ON
    hprt |= (1 << 12);   // PPWR

    // Assert port reset
    hprt |= (1 << 4);    // PRST
    HPRT = hprt;
    delay(50000);        // 50ms

    // Deassert port reset
    hprt &= ~(1 << 4);
    HPRT = hprt;
    delay(100000);       // 100ms

    // Read final state
    hprt = HPRT;

    fb_print("[USB] HPRT = 0x", COLOR_GREEN);
    fb_print_hex(hprt);
    fb_print("\n", COLOR_GREEN);

    if (hprt & 1)
        fb_print("[USB] Root device present (hub)\n", COLOR_GREEN);
    else
        fb_print("[USB] No device (impossible on Pi 3B+)\n", COLOR_RED);

    if (hprt & (1 << 1))
        fb_print("[USB] Port enabled\n", COLOR_GREEN);
    else
        fb_print("[USB] Port NOT enabled\n", COLOR_RED);

    return 0;
}
