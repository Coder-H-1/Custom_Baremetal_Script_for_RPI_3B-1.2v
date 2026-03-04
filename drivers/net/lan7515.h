/* ==========================================================================
 * LAN7515 USB Ethernet Driver — Header
 * Raspberry Pi 3 (BCM2837) — bare-metal
 *
 * The BCM2837's built-in Ethernet is exposed through a LAN7515 USB composite
 * device (VID 0x0424 / PID 0x7515) connected to the on-chip DWC2 USB host.
 * This driver provides raw Ethernet frame send/receive.
 * ========================================================================== */

#pragma once
#include <stdint.h>
#include <stddef.h>

/* ── USB identity ─────────────────────────────────────────────────────────── */
#define LAN7515_VID          0x0424u
#define LAN7515_PID          0x7515u

/* ── Maximum Ethernet frame size ──────────────────────────────────────────── */
#define ETH_MAX_FRAME        1514u   /* 14-byte header + 1500-byte payload    */
#define ETH_FRAME_BUF        (ETH_MAX_FRAME + 8u) /* + LAN7515 command words */

/* ── LAN7515 device register addresses (accessed via USB control transfers) ─ */
#define LAN7515_ID_REV       0x000u
#define LAN7515_INT_STS      0x00Cu
#define LAN7515_HW_CFG       0x010u
#define LAN7515_PMT_CTL      0x014u
#define LAN7515_E2P_CMD      0x040u
#define LAN7515_E2P_DATA     0x044u
#define LAN7515_MAC_CR       0x100u
#define LAN7515_MAC_RX       0x104u
#define LAN7515_MAC_TX       0x108u
#define LAN7515_FLOW         0x10Cu
#define LAN7515_RFE_CTL      0x508u
#define LAN7515_FCT_RX_CTL   0xC00u
#define LAN7515_FCT_TX_CTL   0xC04u
#define LAN7515_RX_FIFO_INF  0xC0Cu
#define LAN7515_TX_FIFO_INF  0xC10u
#define LAN7515_MAC_ADDR_LO  0x118u
#define LAN7515_MAC_ADDR_HI  0x11Cu

/* ── HW_CFG bits ──────────────────────────────────────────────────────────── */
#define HW_CFG_LRST          (1u << 1)   /* soft reset */
#define HW_CFG_MEF           (1u << 4)   /* multiple ethernet frames */
#define HW_CFG_BCE           (1u << 5)   /* burst cap enable */
#define PMT_CTL_READY        (1u << 7)   /* device ready after reset */

/* ── MAC_CR / MAC_RX / MAC_TX enable bits ────────────────────────────────── */
#define MAC_CR_RXEN          (1u << 2)
#define MAC_TX_TXEN          (1u << 0)
#define MAC_RX_RXEN          (1u << 0)
#define FCT_TX_CTL_EN        (1u << 31)
#define FCT_RX_CTL_EN        (1u << 31)

/* ── TX command word A (prepended to every outgoing frame) ───────────────── */
/* Bit 22:0 = frame length; bit 26 = first segment; bit 27 = last segment    */
#define LAN7515_TX_CMD_A(len) \
    (((uint32_t)(len) & 0x000FFFFFu) | (3u << 26))
#define LAN7515_TX_CMD_B(len) \
    ((uint32_t)(len) & 0x000FFFFFu)

/* ── USB Ethernet bulk endpoints ─────────────────────────────────────────── */
#define LAN7515_EP_IN        1u
#define LAN7515_EP_OUT       2u
#define LAN7515_ADDR         1u    /* USB device address after enumeration   */

/* ── Raw Ethernet frame type ─────────────────────────────────────────────── */
typedef struct {
    uint8_t  dst[6];
    uint8_t  src[6];
    uint16_t ethertype;
    uint8_t  payload[1500];
} __attribute__((packed)) eth_frame_t;

/* ── Driver result codes ──────────────────────────────────────────────────── */
#define LAN7515_OK            0
#define LAN7515_ERR_NOTFOUND -1
#define LAN7515_ERR_TIMEOUT  -2
#define LAN7515_ERR_IO       -3

/* ── Public API ───────────────────────────────────────────────────────────── */

/**
 * lan7515_init – enumerate and configure the LAN7515.
 * Returns LAN7515_OK on success, or a negative error code.
 */
int lan7515_init(void);

/**
 * lan7515_send – transmit one raw Ethernet frame.
 * @buf : pointer to frame data (starting at destination MAC)
 * @len : frame length in bytes (without FCS)
 * Returns LAN7515_OK or negative on error.
 */
int lan7515_send(const void *buf, uint16_t len);

/**
 * lan7515_recv – receive one raw Ethernet frame (non-blocking poll).
 * @buf    : buffer to receive into (should be ≥ ETH_MAX_FRAME)
 * @maxlen : maximum bytes to copy
 * Returns number of bytes received, 0 if nothing available, negative on error.
 */
int lan7515_recv(void *buf, uint16_t maxlen);

/**
 * lan7515_get_mac – retrieve the MAC address negotiated during init.
 * @out : 6-byte buffer written with the MAC address
 */
void lan7515_get_mac(uint8_t out[6]);
