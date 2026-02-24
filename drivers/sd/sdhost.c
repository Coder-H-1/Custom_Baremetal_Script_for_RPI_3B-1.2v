#include "../video/framebuffer.h"
#include <stdint.h>


#define EMMC_BASE 0x3F300000UL

// EMMC Registers
#define EMMC_ARG2      (*(volatile uint32_t*)(EMMC_BASE + 0x00))
#define EMMC_BLKSIZE   (*(volatile uint32_t*)(EMMC_BASE + 0x04))
#define EMMC_BLKCNT    (*(volatile uint32_t*)(EMMC_BASE + 0x06))  // Block count
#define EMMC_ARG1      (*(volatile uint32_t*)(EMMC_BASE + 0x08))
#define EMMC_CMDTM     (*(volatile uint32_t*)(EMMC_BASE + 0x0C))
#define EMMC_RESP0     (*(volatile uint32_t*)(EMMC_BASE + 0x10))
#define EMMC_STATUS    (*(volatile uint32_t*)(EMMC_BASE + 0x24))
#define EMMC_CONTROL0  (*(volatile uint32_t*)(EMMC_BASE + 0x28))
#define EMMC_CONTROL1  (*(volatile uint32_t*)(EMMC_BASE + 0x2C))
#define EMMC_INTERRUPT (*(volatile uint32_t*)(EMMC_BASE + 0x30))
#define EMMC_IRPT_MASK (*(volatile uint32_t*)(EMMC_BASE + 0x34))
#define EMMC_DATA      (*(volatile uint32_t*)(EMMC_BASE + 0x20))

// CMDTM flags
#define CMD_RSPNS_NONE  (0 << 16)
#define CMD_RSPNS_48    (1 << 16)
#define CMD_RSPNS_136   (2 << 16)

#define CMD_ISDATA      (1 << 21)
#define CMD_DAT_DIR_RD  (1 << 4)

// Interrupt bits
#define INT_CMD_DONE    (1 << 0)
#define INT_CMD_ERR     (1 << 15)
#define INT_READ_READY  (1 << 5)
#define INT_DATA_ERR    (1 << 20)


// Simple delay
static void delay(int cnt) {
    while (cnt--) asm volatile("nop");
}

// Send a command to the SD card
int sd_send_cmd(uint32_t cmd, uint32_t arg, uint32_t flags) {
    // Wait until CMD line is free
    while (EMMC_STATUS & 1);

    // Clear interrupts
    EMMC_INTERRUPT = 0xFFFFFFFF;

    // Set argument
    EMMC_ARG1 = arg;

    // Send command
    EMMC_CMDTM = (cmd & 0x3F) | flags;

    // Wait for command complete
    while (!(EMMC_INTERRUPT & INT_CMD_DONE)) {
        if (EMMC_INTERRUPT & INT_CMD_ERR) {
            fb_print("[SD] CMD ERROR\n", 0xFF0000);
            return -1;
        }
    }

    // Clear CMD done
    EMMC_INTERRUPT = INT_CMD_DONE;

    return 0;
}

// Minimal sector read
int sd_read_sector(uint32_t lba, uint8_t *buffer) {
    // Set block size & count
    EMMC_BLKSIZE = 512;
    EMMC_BLKCNT  = 1;

    // Send CMD17 (READ_SINGLE_BLOCK)
    if (sd_send_cmd(17, lba, CMD_RSPNS_48 | CMD_ISDATA | CMD_DAT_DIR_RD))
        return -1;

    // Wait until data ready
    while (!(EMMC_INTERRUPT & INT_READ_READY)) {
        if (EMMC_INTERRUPT & INT_DATA_ERR) {
            fb_print("[SD] DATA ERROR\n", 0xFF0000);
            return -1;
        }
    }

    // Read 512 bytes (128 x 32-bit words)
    for (int i = 0; i < 128; i++) {
        ((uint32_t*)buffer)[i] = EMMC_DATA;
    }

    // Clear read-ready interrupt
    EMMC_INTERRUPT = INT_READ_READY;

    return 0;
}


int sd_init(void) {
    // Reset controller
    EMMC_CONTROL1 |= (1 << 24);
    while (EMMC_CONTROL1 & (1 << 24));

    // Enable internal clock
    EMMC_CONTROL1 |= (1 << 0);
    while (!(EMMC_CONTROL1 & (1 << 1)));

    // Enable SD clock
    EMMC_CONTROL1 |= (1 << 2);

    // Enable interrupts
    EMMC_IRPT_MASK = 0xFFFFFFFF;
    EMMC_INTERRUPT = 0xFFFFFFFF;

    // CMD0: GO_IDLE
    if (sd_send_cmd(0, 0, 0))
        return -1;

    // CMD8: Voltage check
    if (sd_send_cmd(8, 0x1AA, CMD_RSPNS_48))
        return -1;

    // ACMD41 loop
    uint32_t ocr;
    do {
        if (sd_send_cmd(55, 0, CMD_RSPNS_48)) return -1;  // APP_CMD
        if (sd_send_cmd(41, 0x40300000, CMD_RSPNS_48)) return -1; // SD_SEND_OP_COND
        ocr = EMMC_RESP0;
    } while (!(ocr & 0x80000000)); // wait until card ready

    // CMD2: ALL_SEND_CID
    if (sd_send_cmd(2, 0, CMD_RSPNS_136)) return -1;

    // CMD3: SEND_RELATIVE_ADDR
    if (sd_send_cmd(3, 0, CMD_RSPNS_48)) return -1;
    uint32_t rca = EMMC_RESP0 >> 16;

    // CMD7: SELECT_CARD
    if (sd_send_cmd(7, rca << 16, CMD_RSPNS_48)) return -1;

    // CMD16: SET_BLOCKLEN 512
    if (sd_send_cmd(16, 512, CMD_RSPNS_48)) return -1;

    return 0;
}
