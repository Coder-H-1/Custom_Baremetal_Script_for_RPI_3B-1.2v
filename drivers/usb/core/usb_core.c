#include "usb_core.h"
#include "../hc/dwc2.h"
#include "../../video/framebuffer.h"
#include "../../../kernel/core/utils.h"

int usb_read_device_descriptor(usb_device_descriptor_t *d) {
    uint8_t buf[18];

    if (dwc2_control_get_descriptor(buf) != 0)
        return -1;

    d->idVendor  = buf[8]  | (buf[9] << 8);
    d->idProduct = buf[10] | (buf[11] << 8);

    return 0;
}

int usb_set_address(uint8_t new_addr)
{
    uint8_t setup[8] = {
        0x00,       // OUT | Standard | Device
        0x05,       // SET_ADDRESS
        new_addr, 0x00,  // wValue
        0x00, 0x00,      // wIndex
        0x00, 0x00       // wLength
    };

    fb_print("[USB] Setting device address...\n", COLOR_GREEN);

    if (dwc2_control_transfer(0, setup, 0, 0, 0) != 0)
        return -1;

    delay_ms(5);  // mandatory delay after SET_ADDRESS

    fb_print("[USB] Device address set\n", COLOR_GREEN);

    return 0;
}