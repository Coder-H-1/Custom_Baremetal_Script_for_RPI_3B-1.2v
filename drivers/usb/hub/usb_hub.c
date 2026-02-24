#include "usb_hub.h"
#include "../hc/dwc2.h"
#include "../../video/framebuffer.h"
#include "../../../kernel/core/utils.h"

#define USB_REQ_GET_DESCRIPTOR 0x06
#define USB_REQ_SET_FEATURE    0x03
#define USB_REQ_CLEAR_FEATURE  0x01

#define USB_DT_HUB 0x29

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

static int hub_get_descriptor(uint8_t hub_addr, usb_hub_descriptor_t *desc) {
    uint8_t setup[8] = {
        0xA0,                     // IN | Class | Device
        USB_REQ_GET_DESCRIPTOR,
        0x00, USB_DT_HUB,         // HUB descriptor
        0x00, 0x00,
        sizeof(usb_hub_descriptor_t), 0x00
    };

    fb_print("[HUB] Get hub descriptor\n", COLOR_GREEN);
    return dwc2_control_transfer(
        hub_addr,
        setup,
        (uint8_t *)desc,
        sizeof(usb_hub_descriptor_t),
        1
    );
}

static void hub_port_power(uint8_t hub_addr, int port) {
    uint8_t setup[8] = {
        0x23,                 // OUT | Class | Other
        USB_REQ_SET_FEATURE,
        HUB_FEATURE_PORT_POWER, 0x00,
        port, 0x00,
        0x00, 0x00
    };

    fb_print("[HUB] Power port ", COLOR_GREEN);
    fb_print_dec(port);
    fb_print("\n", COLOR_GREEN);

    dwc2_control_transfer(hub_addr, setup, 0, 0, 0);
    delay_ms(100);
}

static void hub_port_reset(uint8_t hub_addr, int port) {
    uint8_t setup[8] = {
        0x23,                 // OUT | Class | Other
        USB_REQ_SET_FEATURE,
        HUB_FEATURE_PORT_RESET, 0x00,
        port, 0x00,
        0x00, 0x00
    };

    fb_print("[HUB] Reset port ", COLOR_GREEN);
    fb_print_dec(port);
    fb_print("\n", COLOR_GREEN);

    dwc2_control_transfer(hub_addr, setup, 0, 0, 0);
    delay_ms(50);
}

int usb_hub_init(uint8_t hub_addr) {
    usb_hub_descriptor_t hub;

    fb_print("[HUB] Initializing USB hub\n", COLOR_GREEN);

    if (hub_get_descriptor(hub_addr, &hub) != 0) {
        fb_print("[HUB] Failed to read hub descriptor\n", COLOR_RED);
        return -1;
    }

    fb_print("[HUB] Ports: ", COLOR_GREEN);
    fb_print_dec(hub.bNbrPorts);
    fb_print("\n", COLOR_GREEN);

    for (int port = 1; port <= hub.bNbrPorts; port++) {
        hub_port_power(hub_addr, port);
        hub_port_reset(hub_addr, port);

        fb_print("[HUB] Port ", COLOR_GREEN);
        fb_print_dec(port);
        fb_print(" ready\n", COLOR_GREEN);
    }

    fb_print("[HUB] Hub init done\n", COLOR_GREEN);
    return 0;
}
