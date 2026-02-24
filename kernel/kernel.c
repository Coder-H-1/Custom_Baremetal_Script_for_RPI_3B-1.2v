#include "../drivers/video/framebuffer.h"
#include "../drivers/usb/hc/dwc2.h"
#include "../drivers/usb/hc/usb_msc.h"
#include "../drivers/usb/core/usb_core.h"
#include "../drivers/usb/hub/usb_hub.h"
#include "../drivers/IO/gpio.h"

#include "core/utils.h"





void test(int delay, int type){
    
    gpio_write(17, type);
    delay_ms(delay);
    
}


void kernel_main(void) {
    fb_init(640, 480);
    fb_clear(0);
    fb_print("Kernel start\n", COLOR_GREEN);

    dwc2_init();
    dwc2_port_reset();
    
    dwc2_set_address(1);   // hub
    usb_hub_init(1);


    usb_device_descriptor_t d;

    if (usb_read_device_descriptor(&d) == 0) {
        char h[5];

        h[0] = "0123456789ABCDEF"[(d.idVendor >> 12) & 0xF];
        h[1] = "0123456789ABCDEF"[(d.idVendor >> 8) & 0xF];
        h[2] = "0123456789ABCDEF"[(d.idVendor >> 4) & 0xF];
        h[3] = "0123456789ABCDEF"[d.idVendor & 0xF];
        h[4] = 0;

        fb_print("VID: ", COLOR_GREEN);
        fb_print(h, COLOR_GREEN);

        h[0] = "0123456789ABCDEF"[(d.idProduct >> 12) & 0xF];
        h[1] = "0123456789ABCDEF"[(d.idProduct >> 8) & 0xF];
        h[2] = "0123456789ABCDEF"[(d.idProduct >> 4) & 0xF];
        h[3] = "0123456789ABCDEF"[d.idProduct & 0xF];

        fb_print(" PID: ", COLOR_GREEN);
        fb_print(h, COLOR_GREEN);
        fb_print("\n", COLOR_GREEN);
    }

    usb_read_device_descriptor(&d);

    if (d.bDeviceClass == 0x09) {  
        fb_print("[USB] Root device is HUB\n", COLOR_GREEN);
        usb_hub_init(1);
        test(1, 0);

    }


    while (1);
}



