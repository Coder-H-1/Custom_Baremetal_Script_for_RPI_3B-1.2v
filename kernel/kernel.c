#include "../drivers/video/framebuffer.h"
#include "../drivers/video/mailbox.h"
#include "../drivers/usb/hc/dwc2.h"
#include "../drivers/usb/msc/usb_msc.h"
#include "../drivers/usb/core/usb_core.h"
#include "../drivers/usb/hub/usb_hub.h"
#include "../drivers/IO/gpio.h"

#include "core/utils.h"





void test(int delay, int type){
    
    gpio_write(17, type);
    delay_ms(delay);
    
}

void usb_power_on() {
    mailbox[0] = 8 * 4;           // Buffer size
    mailbox[1] = 0;               // Request code
    mailbox[2] = 0x00028001;      // Tag: Set power state
    mailbox[3] = 8;               // Value buffer size
    mailbox[4] = 8;               // Request/response code
    mailbox[5] = 0x00000003;      // Device ID: USB
    mailbox[6] = 0x00000001;      // State: On
    mailbox[7] = 0;               // End tag
    mailbox_call(MAILBOX_CH_PROP);
}


void kernel_main(void) {
        
    fb_init(640, 480);
    fb_clear(0);
    fb_print("Kernel start\n", COLOR_GREEN);
    fb_print("powering usb\n", COLOR_GREEN);
    usb_power_on();
    delay_ms(20000);
    fb_print("DONE! powering usb\n", COLOR_GREEN);
    
    dwc2_init();
    delay_ms(50000);
    dwc2_port_reset();
    fb_print("[USB] HPRT = ", COLOR_GREEN);
    fb_print_hex((HPRT >> 24) & 0xFF);
    fb_print_hex((HPRT >> 16) & 0xFF);
    fb_print_hex((HPRT >> 8) & 0xFF);
    fb_print_hex(HPRT & 0xFF);
    fb_print("\n", COLOR_GREEN);    
    
    uint8_t buf[8];

    uint8_t setup[8] = {
        0x80,
        0x06,
        0x00, 0x01,
        0x00, 0x00,
        8, 0x00
    };

    dwc2_control_transfer(0, setup, buf, 8, 1);

    fb_print("[USB] Descriptor first byte: ", COLOR_GREEN);
    fb_print_hex(buf[0]);
    
    fb_print("\n", COLOR_GREEN);

    while (1);
}



