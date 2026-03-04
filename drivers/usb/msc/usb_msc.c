#include "usb_msc.h"
#include "../core/usb_core.h"
#include "../hc/dwc2.h"
#include "../../video/framebuffer.h"
#include "../../../kernel/core/utils.h"

static uint32_t global_tag = 0xDEADBEEF;

static int usb_msc_bot_transfer(
    uint8_t addr,
    void *cbw_buf,
    uint8_t cbw_len,
    void *data_buf,
    uint32_t data_len,
    int in
)
{
    usb_msc_cbw_t cbw;
    memset(&cbw, 0, sizeof(cbw));
    cbw.dCBWSignature = MSC_CBW_SIGNATURE;
    cbw.dCBWTag = global_tag++;
    cbw.dCBWDataTransferLength = data_len;
    cbw.bmCBWFlags = in ? 0x80 : 0x00;
    cbw.bCBWLUN = 0;
    cbw.bCBWCBLength = cbw_len;
    memcpy(cbw.CBWCB, cbw_buf, cbw_len);

    // 1. Send CBW (Bulk OUT)
    if (dwc2_bulk_transfer(addr, 1, (uint8_t*)&cbw, sizeof(cbw), 0) != 0) {
        fb_print("[MSC] CBW failed\n", COLOR_RED);
        return -1;
    }

    // 2. Data Stage (Bulk IN or OUT)
    if (data_len > 0) {
        if (dwc2_bulk_transfer(addr, in ? 2 : 1, (uint8_t*)data_buf, data_len, in) != 0) {
            fb_print("[MSC] Data stage failed\n", COLOR_RED);
            return -1;
        }
    }

    // 3. Receive CSW (Bulk IN)
    usb_msc_csw_t csw;
    if (dwc2_bulk_transfer(addr, 2, (uint8_t*)&csw, sizeof(csw), 1) != 0) {
        fb_print("[MSC] CSW failed\n", COLOR_RED);
        return -1;
    }

    if (csw.dCSWSignature != MSC_CSW_SIGNATURE || csw.bCSWStatus != 0) {
        fb_print("[MSC] BOT error: status ", COLOR_RED);
        fb_print_hex(csw.bCSWStatus);
        fb_print("\n", COLOR_RED);
        return -2;
    }

    return 0;
}

int usb_msc_init(uint8_t addr)
{
    scsi_inquiry_data_t inq;
    uint8_t cmd[6] = {SCSI_INQUIRY, 0, 0, 0, sizeof(inq), 0};

    fb_print("[MSC] Inquiry device...\n", COLOR_GREEN);
    if (usb_msc_bot_transfer(addr, cmd, 6, &inq, sizeof(inq), 1) == 0) {
        fb_print("[MSC] Vendor:  ", COLOR_GREEN);
        fb_print(inq.vendor_id, COLOR_GREEN); // Note: vendor_id is not null-terminated!
        fb_print("\n", COLOR_GREEN);
    }

    scsi_read_capacity_10_data_t cap;
    uint8_t cmd_cap[10] = {SCSI_READ_CAPACITY_10, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    
    if (usb_msc_bot_transfer(addr, cmd_cap, 10, &cap, sizeof(cap), 1) == 0) {
        // SCSI is big-endian!
        uint32_t max_lba = (cap.last_lba >> 24) | ((cap.last_lba >> 8) & 0xFF00) | ((cap.last_lba << 8) & 0xFF0000) | (cap.last_lba << 24);
        uint32_t bsize = (cap.block_size >> 24) | ((cap.block_size >> 8) & 0xFF00) | ((cap.block_size << 8) & 0xFF0000) | (cap.block_size << 24);

        fb_print("[MSC] Capacity: ", COLOR_GREEN);
        fb_print_dec((max_lba + 1) * bsize / 1024 / 1024);
        fb_print(" MB\n", COLOR_GREEN);
    }

    return 0;
}

int usb_msc_read(uint8_t addr, uint32_t lba, uint32_t count, void *buffer)
{
    uint8_t cmd[10] = {
        SCSI_READ_10, 0,
        (lba >> 24) & 0xFF, (lba >> 16) & 0xFF, (lba >> 8) & 0xFF, lba & 0xFF,
        0,
        (count >> 8) & 0xFF, count & 0xFF,
        0
    };
    return usb_msc_bot_transfer(addr, cmd, 10, buffer, count * 512, 1);
}
