#ifndef TM4C_SD_DRIVER_H
#define TM4C_SD_DRIVER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool SD_DriverInit(void);
bool SD_DriverMount(void);
uintptr_t SD_DriverWadCacheBase(void);
size_t SD_DriverWadCacheSize(void);
const char *SD_DriverDefaultWadPath(void);

#endif
