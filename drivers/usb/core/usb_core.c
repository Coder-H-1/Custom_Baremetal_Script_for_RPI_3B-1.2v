#include "usb_core.h"
#include "../hc/dwc2.h"
#include "../../video/framebuffer.h"

int usb_read_device_descriptor(usb_device_descriptor_t *d) {
    uint8_t buf[18];

    if (dwc2_control_get_descriptor(buf) != 0)
        return -1;

    d->idVendor  = buf[8]  | (buf[9] << 8);
    d->idProduct = buf[10] | (buf[11] << 8);

    return 0;
}
