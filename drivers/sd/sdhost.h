#ifndef SDHOST_H
#define SDHOST_H

#include <stdint.h>



int sd_init(void);
int sd_read_sector(uint32_t lba, uint8_t *buffer);

#endif
