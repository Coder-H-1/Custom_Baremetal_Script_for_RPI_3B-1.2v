#pragma once
#include <stdint.h>

// Base
#define DWC2_BASE 0x3F980000UL

// Core
#define GAHBCFG   (*(volatile uint32_t*)(DWC2_BASE + 0x008))
#define GUSBCFG   (*(volatile uint32_t*)(DWC2_BASE + 0x00C))
#define GRSTCTL   (*(volatile uint32_t*)(DWC2_BASE + 0x010))

// Host
#define HCFG      (*(volatile uint32_t*)(DWC2_BASE + 0x400))
#define HPRT      (*(volatile uint32_t*)(DWC2_BASE + 0x440))

// Host Channel 0
#define HCCHAR0   (*(volatile uint32_t*)(DWC2_BASE + 0x500))
#define HCTSIZ0   (*(volatile uint32_t*)(DWC2_BASE + 0x510))
#define HCDMA0    (*(volatile uint32_t*)(DWC2_BASE + 0x514))
#define HCINT0    (*(volatile uint32_t*)(DWC2_BASE + 0x508))
#define HCINTMSK0 (*(volatile uint32_t*)(DWC2_BASE + 0x50C))

void dwc2_init(void);
int dwc2_control_get_descriptor(uint8_t *buffer);
int dwc2_port_reset(void);
int dwc2_control_transfer(
    uint8_t dev_addr,
    uint8_t *setup,
    uint8_t *data,
    int len,
    int in
);