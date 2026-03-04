#pragma once
#include <stdint.h>

/* BOT (Bulk-Only Transport) Command Block Wrapper */
typedef struct {
    uint32_t dCBWSignature;
    uint32_t dCBWTag;
    uint32_t dCBWDataTransferLength;
    uint8_t  bmCBWFlags;
    uint8_t  bCBWLUN;
    uint8_t  bCBWCBLength;
    uint8_t  CBWCB[16];
} __attribute__((packed)) usb_msc_cbw_t;

/* BOT Command Status Wrapper */
typedef struct {
    uint32_t dCSWSignature;
    uint32_t dCSWTag;
    uint32_t dCSWDataResidue;
    uint8_t  bCSWStatus;
} __attribute__((packed)) usb_msc_csw_t;

#define MSC_CBW_SIGNATURE 0x43425355
#define MSC_CSW_SIGNATURE 0x53425355

/* SCSI Commands */
#define SCSI_INQUIRY          0x12
#define SCSI_READ_CAPACITY_10 0x25
#define SCSI_READ_10          0x28
#define SCSI_WRITE_10         0x2A
#define SCSI_TEST_UNIT_READY  0x00

typedef struct {
    uint8_t  device_type;
    uint8_t  removable;
    uint8_t  version;
    uint8_t  response_data_format;
    uint8_t  additional_length;
    uint8_t  reserved[3];
    char     vendor_id[8];
    char     product_id[16];
    char     product_revision[4];
} __attribute__((packed)) scsi_inquiry_data_t;

typedef struct {
    uint32_t last_lba;
    uint32_t block_size;
} __attribute__((packed)) scsi_read_capacity_10_data_t;

/* MSC Driver functions */
int usb_msc_init(uint8_t addr);
int usb_msc_read(uint8_t addr, uint32_t lba, uint32_t count, void *buffer);
int usb_msc_write(uint8_t addr, uint32_t lba, uint32_t count, const void *buffer);
