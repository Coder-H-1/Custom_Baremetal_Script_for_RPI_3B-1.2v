#pragma once
#include <stdint.h>

#define USB_DT_HUB 0x29

typedef struct {
    uint16_t wPortStatus;
    uint16_t wPortChange;
} __attribute__((packed)) usb_port_status_t;

/* Port Status Bits */
#define USB_PORT_STAT_CONNECTION    0x0001
#define USB_PORT_STAT_ENABLE        0x0002
#define USB_PORT_STAT_SUSPEND       0x0004
#define USB_PORT_STAT_OVERCURRENT   0x0008
#define USB_PORT_STAT_RESET         0x0010
#define USB_PORT_STAT_POWER         0x0100
#define USB_PORT_STAT_LOW_SPEED     0x0200
#define USB_PORT_STAT_HIGH_SPEED    0x0400

/* Port Change Bits */
#define USB_PORT_STAT_C_CONNECTION  0x0001
#define USB_PORT_STAT_C_ENABLE      0x0002

int usb_hub_init(uint8_t hub_addr);
int usb_hub_poll(uint8_t hub_addr);

/* Exposed helpers for kernel-level port enumeration */
int hub_set_port_feature_ext(uint8_t hub_addr, uint8_t port, uint16_t feature);
int hub_get_port_status_ext(uint8_t hub_addr, uint8_t port, usb_port_status_t *status);
