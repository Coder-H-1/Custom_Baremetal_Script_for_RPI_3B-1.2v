#include "usb_core.h"
#include "../hc/dwc2.h"
#include "../../video/framebuffer.h"
#include "../../../kernel/core/utils.h"

void usb_init_stack(void)
{
    fb_print("[USB] Core Stack Init\n", COLOR_GREEN);
    dwc2_init();
}

int usb_control_msg(
    uint8_t addr,
    uint8_t request_type,
    uint8_t request,
    uint16_t value,
    uint16_t index,
    uint16_t len,
    void *data
)
{
    usb_setup_t setup;
    setup.bmRequestType = request_type;
    setup.bRequest = request;
    setup.wValue = value;
    setup.wIndex = index;
    setup.wLength = len;

    int in = (request_type & 0x80) ? 1 : 0;
    return dwc2_control_transfer(addr, (uint8_t*)&setup, (uint8_t*)data, len, in);
}

int usb_get_descriptor(uint8_t addr, uint8_t type, uint8_t index, void *buf, uint16_t len)
{
    return usb_control_msg(
        addr,
        0x80, // Device to Host | Standard | Device
        USB_REQ_GET_DESCRIPTOR,
        (type << 8) | index,
        0,
        len,
        buf
    );
}

int usb_set_address(uint8_t old_addr, uint8_t new_addr)
{
    int res = usb_control_msg(
        old_addr,
        0x00, // Host to Device | Standard | Device
        USB_REQ_SET_ADDRESS,
        new_addr,
        0,
        0,
        0
    );
    if (res == 0) delay_ms(5);
    return res;
}

int usb_set_configuration(uint8_t addr, uint8_t config)
{
    return usb_control_msg(
        addr,
        0x00, // Host to Device | Standard | Device
        USB_REQ_SET_CONFIGURATION,
        config,
        0,
        0,
        0
    );
}

int usb_enumerate_device(void)
{
    fb_print("[USB] Starting enumeration...\n", COLOR_GREEN);

    dwc2_port_reset();

    // Device starts at address 0
    uint8_t addr = 0;
    static usb_device_descriptor_t desc __attribute__((aligned(16)));

    // 1. Get first 8 bytes of device descriptor to get MaxPacketSize0
    // (We use 8 because all devices support at least 8)
    if (usb_get_descriptor(addr, USB_DESC_DEVICE, 0, &desc, 8) != 0) {
        fb_print("[USB] Failed to get device descriptor (initial)\n", COLOR_RED);
        return -1;
    }

    fb_print("[USB] MaxPacketSize0: ", COLOR_GREEN);
    fb_print_hex(desc.bMaxPacketSize0);
    fb_print("\n", COLOR_GREEN);

    // 2. Set Address (we'll use 1 for now as a simple example)
    if (usb_set_address(0, 1) != 0) {
        fb_print("[USB] Failed to set address\n", COLOR_RED);
        return -1;
    }
    addr = 1;

    // 3. Get full device descriptor
    if (usb_get_descriptor(addr, USB_DESC_DEVICE, 0, &desc, sizeof(desc)) != 0) {
        fb_print("[USB] Failed to get full device descriptor\n", COLOR_RED);
        return -1;
    }

    fb_print("[USB] Vendor ID:  ", COLOR_GREEN);
    fb_print_hex16(desc.idVendor);
    fb_print("\n", COLOR_GREEN);
    fb_print("[USB] Product ID: ", COLOR_GREEN);
    fb_print_hex16(desc.idProduct);
    fb_print("\n", COLOR_GREEN);

    return 0;
}