#include "usb_hub.h"
#include "../core/usb_core.h"
#include "../../video/framebuffer.h"
#include "../../../kernel/core/utils.h"

#define HUB_CLASS_REQ_GET_DESCRIPTOR 0x06
#define HUB_CLASS_REQ_SET_FEATURE    0x03
#define HUB_CLASS_REQ_CLEAR_FEATURE  0x01
#define HUB_CLASS_REQ_GET_STATUS     0x00

#define HUB_FEATURE_PORT_POWER 8
#define HUB_FEATURE_PORT_RESET 4

typedef struct {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bNbrPorts;
    uint16_t wHubCharacteristics;
    uint8_t  bPwrOn2PwrGood;
    uint8_t  bHubContrCurrent;
} __attribute__((packed)) usb_hub_descriptor_t;

static int hub_get_descriptor(uint8_t hub_addr, usb_hub_descriptor_t *desc)
{
    return usb_control_msg(
        hub_addr,
        0xA0, // IN | Class | Device
        HUB_CLASS_REQ_GET_DESCRIPTOR,
        (USB_DT_HUB << 8),
        0,
        sizeof(usb_hub_descriptor_t),
        desc
    );
}

static int hub_set_port_feature(uint8_t hub_addr, uint8_t port, uint16_t feature)
{
    return usb_control_msg(
        hub_addr,
        0x23, // OUT | Class | Other
        HUB_CLASS_REQ_SET_FEATURE,
        feature,
        port,
        0,
        0
    );
}

static int hub_get_port_status(uint8_t hub_addr, uint8_t port, usb_port_status_t *status)
{
    return usb_control_msg(
        hub_addr,
        0xA3, // IN | Class | Other
        HUB_CLASS_REQ_GET_STATUS,
        0,
        port,
        sizeof(usb_port_status_t),
        status
    );
}

int usb_hub_init(uint8_t hub_addr)
{
    usb_hub_descriptor_t hub;
    fb_print("[HUB] Initializing hub at addr ", COLOR_GREEN);
    fb_print_hex(hub_addr);
    fb_print("\n", COLOR_GREEN);

    if (hub_get_descriptor(hub_addr, &hub) != 0) {
        fb_print("[HUB] Failed to read hub descriptor\n", COLOR_RED);
        return -1;
    }

    fb_print("[HUB] Detected ", COLOR_GREEN);
    fb_print_dec(hub.bNbrPorts);
    fb_print(" ports\n", COLOR_GREEN);

    for (int port = 1; port <= hub.bNbrPorts; port++) {
        hub_set_port_feature(hub_addr, port, HUB_FEATURE_PORT_POWER);
        delay_ms(hub.bPwrOn2PwrGood * 2);
    }

    return 0;
}

int usb_hub_poll(uint8_t hub_addr)
{
    usb_hub_descriptor_t hub;
    if (hub_get_descriptor(hub_addr, &hub) != 0) return -1;

    for (int port = 1; port <= hub.bNbrPorts; port++) {
        usb_port_status_t status;
        if (hub_get_port_status(hub_addr, port, &status) == 0) {
            if (status.wPortChange & USB_PORT_STAT_C_CONNECTION) {
                if (status.wPortStatus & USB_PORT_STAT_CONNECTION) {
                    fb_print("[HUB] Port ", COLOR_GREEN);
                    fb_print_dec(port);
                    fb_print(" device connected!\n", COLOR_GREEN);

                    // Reset port to start enumeration
                    hub_set_port_feature(hub_addr, port, HUB_FEATURE_PORT_RESET);
                    delay_ms(50);
                    
                    // In a real stack, we would now call usb_enumerate_device()
                    // for the device on this port.
                } else {
                    fb_print("[HUB] Port ", COLOR_GREEN);
                    fb_print_dec(port);
                    fb_print(" device disconnected\n", COLOR_GREEN);
                }
                // Clear the change bit? Standard hubs require clearing feature 16 (C_PORT_CONNECTION)
                usb_control_msg(hub_addr, 0x23, HUB_CLASS_REQ_CLEAR_FEATURE, 16, port, 0, 0);
            }
        }
    }
    return 0;
}

/* ── Public wrappers for kernel-level port enumeration ──────────────────── */
int hub_set_port_feature_ext(uint8_t hub_addr, uint8_t port, uint16_t feature) {
    return hub_set_port_feature(hub_addr, port, feature);
}

int hub_get_port_status_ext(uint8_t hub_addr, uint8_t port, usb_port_status_t *status) {
    return hub_get_port_status(hub_addr, port, status);
}
