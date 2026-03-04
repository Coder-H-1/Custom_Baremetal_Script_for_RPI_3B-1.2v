/* ==========================================================================
 * FAT32 Driver Stub
 * Full implementation pending — shell 'ls' and 'cat' commands will report
 * "filesystem not mounted" until this is complete.
 * ========================================================================== */

#include "fat32.h"

int fat32_init(void) {
    return -1;   /* not yet implemented */
}

int fat32_read_file(const char *path, void *buf, uint32_t maxlen) {
    (void)path; (void)buf; (void)maxlen;
    return -1;
}
