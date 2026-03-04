/* ==========================================================================
 * Project NEXUS — Kernel Entry Point
 * Raspberry Pi 3 (BCM2837, AArch64 bare-metal)
 * ========================================================================== */

#include "../drivers/video/framebuffer.h"
#include "../drivers/video/mailbox.h"
#include "../drivers/uart/uart.h"
#include "../drivers/usb/hc/dwc2.h"
#include "../drivers/usb/core/usb_core.h"
#include "../drivers/usb/hub/usb_hub.h"
#include "../drivers/net/lan7515.h"

#include "core/utils.h"

/* ── USB OTG power-on via VideoCore mailbox ─────────────────────────────── */
static void usb_power_on(void) {
    mailbox[0] = 8 * 4;
    mailbox[1] = 0;
    mailbox[2] = 0x00028001;   /* Tag: Set power state */
    mailbox[3] = 8;
    mailbox[4] = 8;
    mailbox[5] = 0x00000003;   /* Device: USB */
    mailbox[6] = 0x00000001;   /* State: ON */
    mailbox[7] = 0;
    mailbox_call(MAILBOX_CH_PROP);
}

/* ── USB Discovery ──────────────────────────────────────────────────────── */
static void usb_discovery(void) {
    fb_print("[BOOT] Starting USB discovery...\n", COLOR_GREEN);

    usb_device_descriptor_t desc;
    if (usb_get_descriptor(1, USB_DESC_DEVICE, 0, &desc, sizeof(desc)) != 0) {
        fb_print("[BOOT] Root device discovery failed\n", COLOR_RED);
        return;
    }

    fb_print("[USB] Root: VID=0x", COLOR_GREEN);
    fb_print_hex16(desc.idVendor);
    fb_print(" PID=0x", COLOR_GREEN);
    fb_print_hex16(desc.idProduct);
    fb_print(" Class=0x", COLOR_GREEN);
    fb_print_hex(desc.bDeviceClass);
    fb_print("\n", COLOR_GREEN);

    usb_set_configuration(1, 1);
    delay_ms(50);

    /* If it's a hub (class 9), look at its ports */
    if (desc.bDeviceClass == 9) {
        fb_print("[USB] Root is HUB, scanning ports...\n", COLOR_GREEN);
        usb_hub_init(1);
        delay_ms(200);

        for (int port = 1; port <= 4; port++) {
            usb_port_status_t ps;
            if (hub_get_port_status_ext(1, port, &ps) == 0) {
                if (ps.wPortStatus & USB_PORT_STAT_CONNECTION) {
                    fb_print("[USB] Port ", COLOR_GREEN);
                    fb_print_dec(port);
                    fb_print(": Device detected!\n", COLOR_GREEN);
                    
                    /* Simple reset to see if it responds */
                    hub_set_port_feature_ext(1, port, 4 /* RESET */);
                    delay_ms(100);
                }
            }
        }
    }
}

/* ── Kernel entry ───────────────────────────────────────────────────────── */
void kernel_main(void) {

    /* 1. FB init */
    fb_init(640, 480);
    fb_clear(0);
    fb_print("[BOOT 1/6] Framebuffer OK\n", COLOR_GREEN);

    /* 2. Debug UART */
    uart_init();
    fb_print("[BOOT 2/6] UART OK\n", COLOR_GREEN);

    /* 3. USB Power */
    fb_print("[BOOT 3/6] Powering USB...\n", COLOR_GREEN);
    usb_power_on();
    delay_ms(500);

    /* 4. DWC2 */
    fb_print("[BOOT 4/6] Init DWC2...\n", COLOR_GREEN);
    dwc2_init();
    delay_ms(100);

    /* 5. Port Reset */
    fb_print("[BOOT 5/6] USB Port Reset...\n", COLOR_GREEN);
    dwc2_port_reset();
    delay_ms(200);

    /* 6. Discovery */
    fb_print("[BOOT 6/6] Discovering Devices...\n", COLOR_GREEN);
    usb_discovery();

    fb_print("[BOOT] All init complete.\n", 0x00BFFF);
    
    while (1) {
    }
}
