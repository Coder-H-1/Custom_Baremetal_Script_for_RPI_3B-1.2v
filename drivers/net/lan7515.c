/* ==========================================================================
 * LAN7515 USB Ethernet Driver — Implementation
 * Raspberry Pi 3 (BCM2837) bare-metal
 *
 * The LAN7515 is enumerated as a USB device. We configure it via USB control
 * transfers (vendor class, using its internal register interface) and transfer
 * Ethernet frames over USB bulk endpoints.
 *
 * USB bulk transfers use the DWC2 host controller (dwc2_bulk_transfer).
 * Control transfers use usb_control_msg from usb_core.
 * ========================================================================== */

#include "lan7515.h"
#include "../usb/core/usb_core.h"
#include "../usb/hc/dwc2.h"
#include "../../kernel/core/utils.h"
#include "../video/framebuffer.h"

/* ── Static MAC address (used if OTP read fails) ──────────────────────────── */
static uint8_t g_mac[6] = { 0xB8, 0x27, 0xEB, 0x00, 0x00, 0x01 };

/* ── Scratch buffer for TX (frame + 8-byte command header) ───────────────── */
static uint8_t tx_buf[ETH_FRAME_BUF];

/* ── Scratch buffer for RX (frame + 4-byte status word) ─────────────────── */
static uint8_t rx_buf[ETH_FRAME_BUF + 4];

/* =========================================================================
 * Low-level register read/write via USB control transfer
 *
 * LAN7515 uses a vendor-class control interface:
 *   bmRequestType = 0xA1  (IN  | Class | Interface) for reads
 *   bmRequestType = 0x21  (OUT | Class | Interface) for writes
 *   bRequest      = 0xA1  reads  /  0xA0  writes (vendor commands)
 *   wValue        = 0
 *   wIndex        = register address
 *   wLength       = 4 (registers are 32-bit)
 * ========================================================================= */

#define LAN7515_CTRL_READ_TYPE   0xA1u
#define LAN7515_CTRL_WRITE_TYPE  0x21u
#define LAN7515_CTRL_READ_REQ    0xA1u
#define LAN7515_CTRL_WRITE_REQ   0xA0u

static int lan_reg_read(uint16_t reg, uint32_t *val) {
    int r = usb_control_msg(
        LAN7515_ADDR,
        LAN7515_CTRL_READ_TYPE,
        LAN7515_CTRL_READ_REQ,
        0,          /* wValue */
        reg,        /* wIndex = register address */
        4,          /* wLength */
        val
    );
    return r;
}

static int lan_reg_write(uint16_t reg, uint32_t val) {
    return usb_control_msg(
        LAN7515_ADDR,
        LAN7515_CTRL_WRITE_TYPE,
        LAN7515_CTRL_WRITE_REQ,
        0,
        reg,
        4,
        &val
    );
}

/* Busy-wait on a register bit (with timeout) */
static int lan_wait_bit_clear(uint16_t reg, uint32_t mask, uint32_t timeout_ms) {
    uint32_t val;
    while (timeout_ms--) {
        if (lan_reg_read(reg, &val) != 0) return LAN7515_ERR_IO;
        if (!(val & mask)) return LAN7515_OK;
        delay_ms(1);
    }
    return LAN7515_ERR_TIMEOUT;
}

static int lan_wait_bit_set(uint16_t reg, uint32_t mask, uint32_t timeout_ms) {
    uint32_t val;
    while (timeout_ms--) {
        if (lan_reg_read(reg, &val) != 0) return LAN7515_ERR_IO;
        if (val & mask) return LAN7515_OK;
        delay_ms(1);
    }
    return LAN7515_ERR_TIMEOUT;
}

/* =========================================================================
 * lan7515_init
 * ========================================================================= */
int lan7515_init(void) {
    int r;
    uint32_t val;

    /* ── 1. Verify chip is present by reading ID_REV ─────────────────────── */
    if (lan_reg_read(LAN7515_ID_REV, &val) != 0) {
        fb_print("[LAN7515] Not found (no response on USB)\n", COLOR_RED);
        return LAN7515_ERR_NOTFOUND;
    }
    /* Upper 16 bits = chip ID; LAN7515 = 0x7500 */
    if ((val >> 16) != 0x7500u && (val >> 16) != 0x7505u) {
        fb_print("[LAN7515] Unexpected chip ID: 0x", COLOR_RED);
        fb_print_hex32(val);
        fb_print("\n", COLOR_RED);
        /* Continue anyway – might still work on similar variants */
    }

    /* ── 2. Soft reset ───────────────────────────────────────────────────── */
    r = lan_reg_write(LAN7515_HW_CFG, HW_CFG_LRST);
    if (r) return r;
    r = lan_wait_bit_clear(LAN7515_HW_CFG, HW_CFG_LRST, 200);
    if (r) { fb_print("[LAN7515] Soft reset timeout\n", COLOR_RED); return r; }

    /* ── 3. Wait for device-ready ────────────────────────────────────────── */
    r = lan_wait_bit_set(LAN7515_PMT_CTL, PMT_CTL_READY, 200);
    if (r) { fb_print("[LAN7515] PMT ready timeout\n", COLOR_RED); return r; }

    /* ── 4. Enable multiple-frame & burst-cap in HW_CFG ─────────────────── */
    lan_reg_read(LAN7515_HW_CFG, &val);
    val |= HW_CFG_MEF | HW_CFG_BCE;
    lan_reg_write(LAN7515_HW_CFG, val);

    /* ── 5. Read MAC address from OTP (E2P) ──────────────────────────────── */
    /* Issue OTP read: command = 0x00000000 | EEPROM_READ (bit 28) at addr 0 */
    lan_reg_read(LAN7515_MAC_ADDR_LO, &val);
    if (val != 0xFFFFFFFFu && val != 0x00000000u) {
        g_mac[0] = (uint8_t)(val >>  0);
        g_mac[1] = (uint8_t)(val >>  8);
        g_mac[2] = (uint8_t)(val >> 16);
        g_mac[3] = (uint8_t)(val >> 24);
        uint32_t hi;
        lan_reg_read(LAN7515_MAC_ADDR_HI, &hi);
        g_mac[4] = (uint8_t)(hi >> 0);
        g_mac[5] = (uint8_t)(hi >> 8);
    } /* else keep the static fallback MAC */

    /* ── 6. Write MAC address into device ────────────────────────────────── */
    uint32_t mac_lo = ((uint32_t)g_mac[0])       |
                      ((uint32_t)g_mac[1] <<  8)  |
                      ((uint32_t)g_mac[2] << 16)  |
                      ((uint32_t)g_mac[3] << 24);
    uint32_t mac_hi = ((uint32_t)g_mac[4])        |
                      ((uint32_t)g_mac[5] <<  8);
    lan_reg_write(LAN7515_MAC_ADDR_LO, mac_lo);
    lan_reg_write(LAN7515_MAC_ADDR_HI, mac_hi);

    /* ── 7. Reception filter: accept all unicast & broadcast ─────────────── */
    lan_reg_write(LAN7515_RFE_CTL, 0x00000007u);  /* DPF | BCST | UCST */

    /* ── 8. Enable FIFO controllers ──────────────────────────────────────── */
    lan_reg_write(LAN7515_FCT_RX_CTL, FCT_RX_CTL_EN);
    lan_reg_write(LAN7515_FCT_TX_CTL, FCT_TX_CTL_EN);

    /* ── 9. Enable MAC TX path ───────────────────────────────────────────── */
    lan_reg_read(LAN7515_MAC_TX, &val);
    val |= MAC_TX_TXEN;
    lan_reg_write(LAN7515_MAC_TX, val);

    /* ── 10. Enable MAC RX path ──────────────────────────────────────────── */
    lan_reg_read(LAN7515_MAC_RX, &val);
    val |= MAC_RX_RXEN;
    lan_reg_write(LAN7515_MAC_RX, val);

    fb_print("[LAN7515] Init OK. MAC: ", COLOR_GREEN);
    for (int i = 0; i < 6; i++) {
        fb_print_hex(g_mac[i]);
        if (i < 5) fb_print(":", COLOR_GREEN);
    }
    fb_print("\n", COLOR_GREEN);

    return LAN7515_OK;
}

/* =========================================================================
 * lan7515_send  – transmit one Ethernet frame via USB bulk OUT
 *
 * The LAN7515 requires a two-word (8-byte) TX command header before the frame:
 *   Word 0 (TX_CMD_A): frame length | first-segment | last-segment flags
 *   Word 1 (TX_CMD_B): frame length (checksum offload / tag info, unused = len)
 * ========================================================================= */
int lan7515_send(const void *buf, uint16_t len) {
    if (len > ETH_MAX_FRAME) return LAN7515_ERR_IO;

    /* Build TX header */
    uint32_t cmd_a = LAN7515_TX_CMD_A(len);
    uint32_t cmd_b = LAN7515_TX_CMD_B(len);
    tx_buf[0] = (uint8_t)(cmd_a >>  0);
    tx_buf[1] = (uint8_t)(cmd_a >>  8);
    tx_buf[2] = (uint8_t)(cmd_a >> 16);
    tx_buf[3] = (uint8_t)(cmd_a >> 24);
    tx_buf[4] = (uint8_t)(cmd_b >>  0);
    tx_buf[5] = (uint8_t)(cmd_b >>  8);
    tx_buf[6] = (uint8_t)(cmd_b >> 16);
    tx_buf[7] = (uint8_t)(cmd_b >> 24);

    memcpy(tx_buf + 8, buf, len);

    /* Issue bulk OUT transfer.  dwc2_bulk_transfer(addr, ep, data, len, in) */
    int r = dwc2_bulk_transfer(LAN7515_ADDR, LAN7515_EP_OUT,
                               tx_buf, (uint32_t)(len + 8), 0 /* OUT */);
    return (r == 0) ? LAN7515_OK : LAN7515_ERR_IO;
}

/* =========================================================================
 * lan7515_recv  – poll for a received Ethernet frame via USB bulk IN
 *
 * The LAN7515 prepends a 4-byte RX status word to each frame.  If no data is
 * available the bulk IN transfer returns 0 bytes (NAK → short packet).
 * ========================================================================= */
int lan7515_recv(void *buf, uint16_t maxlen) {
    int got = dwc2_bulk_transfer(LAN7515_ADDR, LAN7515_EP_IN,
                                 rx_buf, (uint32_t)sizeof(rx_buf), 1 /* IN */);
    if (got <= 4) return 0;   /* only status word or nothing */

    /* RX status word (bits 16:0 = frame length including status word) */
    uint32_t status = (uint32_t)rx_buf[0]        |
                      ((uint32_t)rx_buf[1] <<  8) |
                      ((uint32_t)rx_buf[2] << 16) |
                      ((uint32_t)rx_buf[3] << 24);
    uint32_t frame_len = status & 0x3FFFu;
    if (frame_len < 4 || frame_len > (uint32_t)(got - 4))
        frame_len = (uint32_t)(got - 4);

    uint16_t copy = (frame_len <= maxlen) ? (uint16_t)frame_len : maxlen;
    memcpy(buf, rx_buf + 4, copy);
    return (int)copy;
}

/* =========================================================================
 * lan7515_get_mac
 * ========================================================================= */
void lan7515_get_mac(uint8_t out[6]) {
    memcpy(out, g_mac, 6);
}
