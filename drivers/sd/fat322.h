#pragma once
#include <stdint.h>

int fat32_init(void);
int fat32_read_file(const char *path, void *buf, uint32_t maxlen);
