#include "sdcard.h"
#include "gb_ll_sdcard.h"
#include <sys/stat.h>
#include <errno.h>

bool sd_init() {
    gb_ll_sd_init();     // monte la carte sur /sdcard (MOUNT_POINT)
    return true;
}

bool sd_mkdir(const char* path) {
    if (!path) return false;
    if (mkdir(path, 0777) == 0) return true;
    return errno == EEXIST;   // deja present = succes
}

bool sd_exists(const char* path) {
    struct stat st;
    return path && stat(path, &st) == 0;
}
