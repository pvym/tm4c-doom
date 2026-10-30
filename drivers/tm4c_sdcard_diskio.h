/*
 * tm4c_sdcard_diskio.h - propojení tm4c_sdcard s FatFs (diskio.h, jednotka 0).
 *
 * Volitelné: kompilujte jen s FatFs. Před f_mount() zavolejte
 *   Tm4cSdDiskioAttach(&sd);   // sd musí být po Tm4cSdBusInit()
 * disk_initialize() pak provede Tm4cSdCardInit(). Zápis není podporován.
 */
#ifndef TM4C_SDCARD_DISKIO_H
#define TM4C_SDCARD_DISKIO_H

#include "tm4c_sdcard.h"

void Tm4cSdDiskioAttach(tm4c_sdcard_t *sd);

#endif
