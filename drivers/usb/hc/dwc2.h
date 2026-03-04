#pragma once
#include <stdint.h>

#define DWC2_BASE 0x3F980000UL

/* Core Registers */
#define GOTGCTL   (*(volatile uint32_t*)(DWC2_BASE + 0x000))
#define GOTGINT   (*(volatile uint32_t*)(DWC2_BASE + 0x004))
#define GAHBCFG   (*(volatile uint32_t*)(DWC2_BASE + 0x008))
#define GUSBCFG   (*(volatile uint32_t*)(DWC2_BASE + 0x00C))
#define GRSTCTL   (*(volatile uint32_t*)(DWC2_BASE + 0x010))
#define GINTSTS   (*(volatile uint32_t*)(DWC2_BASE + 0x014))
#define GINTMSK   (*(volatile uint32_t*)(DWC2_BASE + 0x018))
#define GRXSTSR   (*(volatile uint32_t*)(DWC2_BASE + 0x01C))
#define GRXFSIZ   (*(volatile uint32_t*)(DWC2_BASE + 0x024))
#define GNPTXFSIZ (*(volatile uint32_t*)(DWC2_BASE + 0x028))

/* Host Registers */
#define HCFG      (*(volatile uint32_t*)(DWC2_BASE + 0x400))
#define HFIR      (*(volatile uint32_t*)(DWC2_BASE + 0x404))
#define HFNUM     (*(volatile uint32_t*)(DWC2_BASE + 0x408))
#define HPTXFSIZ  (*(volatile uint32_t*)(DWC2_BASE + 0x410))
#define HAINT     (*(volatile uint32_t*)(DWC2_BASE + 0x414))
#define HAINTMSK  (*(volatile uint32_t*)(DWC2_BASE + 0x418))
#define HPRT      (*(volatile uint32_t*)(DWC2_BASE + 0x440))

/* Host Channel Registers */
#define HCCHAR(n) (*(volatile uint32_t*)(DWC2_BASE + 0x500 + (n)*0x20))
#define HCINT(n)  (*(volatile uint32_t*)(DWC2_BASE + 0x508 + (n)*0x20))
#define HCTSIZ(n) (*(volatile uint32_t*)(DWC2_BASE + 0x510 + (n)*0x20))
#define HCDMA(n)  (*(volatile uint32_t*)(DWC2_BASE + 0x514 + (n)*0x20))

/* --- Register Bits --- */
#define GAHBCFG_GLBLINTRMSK (1 << 0)
#define GAHBCFG_DMAENA      (1 << 5)

#define GUSBCFG_PHYIF       (1 << 3)
#define GUSBCFG_FORCEDEVMODE  (1 << 29)
#define GUSBCFG_FORCEHOSTMODE (1 << 30)

#define GRSTCTL_CSFTRST     (1 << 0)
#define GRSTCTL_RXFFLSH     (1 << 4)
#define GRSTCTL_TXFFLSH     (1 << 5)
#define GRSTCTL_TXFNUM_ALL  (0x10 << 6)
#define GRSTCTL_AHBIDLE     (1 << 31)

/* Bitfields */
#define GINTSTS_CURMODE (1 << 0)
#define GINTSTS_SOF     (1 << 3)
#define GINTSTS_RXFLVL  (1 << 4)
#define GINTSTS_HCHINT  (1 << 25)
#define GINTSTS_PRTINT  (1 << 24)

#define HPRT_PRTCONNSTS (1 << 0)
#define HPRT_PRTCONNDET (1 << 1)
#define HPRT_PRTENA     (1 << 2)
#define HPRT_PRTENCHNG  (1 << 3)
#define HPRT_PRTOVRCURRCHNG (1 << 5)
#define HPRT_PRTRST     (1 << 8)
#define HPRT_PRTPWR     (1 << 12)
#define HPRT_PRTSPD_MASK (3 << 17)

#define HPRT_W1C_MASK (HPRT_PRTCONNDET | HPRT_PRTENCHNG | HPRT_PRTOVRCURRCHNG)

#define HCCHAR_MPS_MASK     0x7FF
#define HCCHAR_EPNUM_MASK   (0xF << 11)
#define HCCHAR_EPDIR_IN     (1 << 15)
#define HCCHAR_LSDEV        (1 << 17)
#define HCCHAR_EPTYPE_CTRL  (0 << 18)
#define HCCHAR_EPTYPE_ISOC  (1 << 18)
#define HCCHAR_EPTYPE_BULK  (2 << 18)
#define HCCHAR_EPTYPE_INTR  (3 << 18)
#define HCCHAR_DEVADDR_MASK (0x7F << 22)
#define HCCHAR_CHDIS        (1 << 30)
#define HCCHAR_CHENA        (1 << 31)

#define HCTSIZ_XFERSIZE_MASK 0x7FFFF
#define HCTSIZ_PKTCNT_MASK   (0x3FF << 19)
#define HCTSIZ_PID_SETUP     (3 << 29)
#define HCTSIZ_PID_DATA0     (0 << 29)
#define HCTSIZ_PID_DATA1     (2 << 29)
#define HCTSIZ_PID_DATA2     (1 << 29)
#define HCTSIZ_PID_MDATA     (3 << 29)

#define HCINT_XFERC          (1 << 0)
#define HCINT_CHHLTD         (1 << 1)
#define HCINT_AHBERR         (1 << 2)
#define HCINT_STALL          (1 << 3)
#define HCINT_NAK            (1 << 4)
#define HCINT_ACK            (1 << 5)
#define HCINT_NYET           (1 << 6)
#define HCINT_XACTERR        (1 << 7)
#define HCINT_BBLERR         (1 << 8)
#define HCINT_FRMOVRUN       (1 << 9)
#define HCINT_DATATGLERR     (1 << 10)

int dwc2_init(void);
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

int dwc2_interrupt_transfer(
    uint8_t dev_addr,
    uint8_t ep,
    uint8_t *data,
    uint32_t len,
    int in
);