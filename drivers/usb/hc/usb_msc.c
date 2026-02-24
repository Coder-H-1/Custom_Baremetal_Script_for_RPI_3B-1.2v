// usb_msc.c
#include "dwc2.h"
#include "../../video/framebuffer.h"
#include "../../../kernel/core/utils.h"

#define MAX_CONFIG_DESC_SIZE 256

typedef struct {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t wTotalLength;
    uint8_t  bNumInterfaces;
    uint8_t  bConfigurationValue;
    uint8_t  iConfiguration;
    uint8_t  bmAttributes;
    uint8_t  bMaxPower;
} __attribute__((packed)) usb_config_descriptor_t;

typedef struct {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bEndpointAddress;
    uint8_t  bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t  bInterval;
} __attribute__((packed)) usb_endpoint_descriptor_t;

static uint8_t bulk_in_ep  = 0;
static uint8_t bulk_out_ep = 0;


static void delay(int ms) {
    volatile int c = ms * 1000; // adjust for speed
    while(c--) asm volatile("nop");
}

#define HCINT_XFRC   (1 << 1)
#define HCINT_STALL  (1 << 3)
#define HCINT_NAK    (1 << 4)
#define HCINT_TXERR  (1 << 7)


// Control transfer: setup 8 bytes, optional data, direction IN=1/OUT=0
int dwc2_control_transfer(
    uint8_t dev_addr,
    uint8_t *setup,
    uint8_t *data,
    int len,
    int in
) {
    const int EP0_MAX_PKT = 8;
    uint32_t hcchar, hctsiz;
    int pktcnt;

    /* =======================
     * 1. SETUP STAGE
     * ======================= */
    fb_print("[USB] SETUP stage\n", COLOR_GREEN);

    HCINT0 = 0xFFFFFFFF;
    HCDMA0 = (uint32_t)setup;

    hctsiz =
        (8 & 0x7FFFF) |   // XferSize
        (1 << 19)     |   // PKTCNT
        (0 << 29);        // PID = SETUP
    HCTSIZ0 = hctsiz;

    hcchar =
        (dev_addr << 22) |
        (0 << 15) |      // OUT
        (0 << 11) |      // EP0
        (1 << 31);       // CHENA
    HCCHAR0 = hcchar;

    while (!(HCINT0 & HCINT_XFRC));
    HCINT0 = 0xFFFFFFFF;

    /* =======================
     * 2. DATA STAGE (optional)
     * ======================= */
    if (len > 0) {
        fb_print("[USB] DATA stage\n", COLOR_GREEN);

        pktcnt = (len + EP0_MAX_PKT - 1) / EP0_MAX_PKT;

        HCDMA0 = (uint32_t)data;

        hctsiz =
            (len & 0x7FFFF) |
            (pktcnt << 19) |
            (1 << 29);          // PID = DATA1
        HCTSIZ0 = hctsiz;

        hcchar =
            (dev_addr << 22) |
            (in ? (1 << 15) : 0) |
            (0 << 11) |
            (1 << 31);
        HCCHAR0 = hcchar;

        while (!(HCINT0 & HCINT_XFRC));
        HCINT0 = 0xFFFFFFFF;
    }

    /* =======================
     * 3. STATUS STAGE
     * ======================= */
    fb_print("[USB] STATUS stage\n", COLOR_GREEN);

    HCDMA0 = 0;

    hctsiz =
        (0 << 0)  |     // zero-length
        (1 << 19) |     // PKTCNT = 1
        (1 << 29);      // PID = DATA1
    HCTSIZ0 = hctsiz;

    hcchar =
        (dev_addr << 22) |
        (in ? 0 : (1 << 15)) | // opposite of DATA
        (0 << 11) |
        (1 << 31);
    HCCHAR0 = hcchar;

    while (!(HCINT0 & HCINT_XFRC));
    HCINT0 = 0xFFFFFFFF;

    fb_print("[USB] Control transfer done\n", COLOR_GREEN);
    return 0;
}
