#pragma once
#include <stdint.h>

#define DWC2_BASE 0x3F980000UL

/* Core Registers */
#define GAHBCFG   (*(volatile uint32_t*)(DWC2_BASE + 0x008))
#define GUSBCFG   (*(volatile uint32_t*)(DWC2_BASE + 0x00C))
#define GRSTCTL   (*(volatile uint32_t*)(DWC2_BASE + 0x010))

#define GINTSTS   (*(volatile uint32_t *)(DWC2_BASE + 0x014))
#define HFNUM     (*(volatile uint32_t *)(DWC2_BASE + 0x408))

/* Host Registers */
#define HCFG      (*(volatile uint32_t*)(DWC2_BASE + 0x400))
#define HPRT      (*(volatile uint32_t*)(DWC2_BASE + 0x440))


/* Host Channel 0 */
#define HCCHAR0   (*(volatile uint32_t*)(DWC2_BASE + 0x500))
#define HCTSIZ0   (*(volatile uint32_t*)(DWC2_BASE + 0x510))
#define HCDMA0    (*(volatile uint32_t*)(DWC2_BASE + 0x514))
#define HCINT0    (*(volatile uint32_t*)(DWC2_BASE + 0x508))

#define HPRT_W1C_MASK ((1 << 1) | (1 << 3) | (1 << 5))

#define GINTSTS_CMOD   (1 << 0)

#define PCGCTL (*(volatile uint32_t*)(DWC2_BASE + 0xE00))
    
void dwc2_init(void);
int  dwc2_port_reset(void);

int dwc2_control_transfer(
    uint8_t dev_addr,
    uint8_t *setup,
    uint8_t *data,
    uint16_t len,
    int in
);

int dwc2_bulk_transfer(
    uint8_t dev_addr,
    uint8_t ep,
    uint8_t *data,
    uint32_t len,
    int in
);