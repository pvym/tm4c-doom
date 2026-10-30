#include "tm4c_sdcard_diskio.h"

#include <stddef.h>

#include "diskio.h"

static tm4c_sdcard_t *s_sd;

void Tm4cSdDiskioAttach(tm4c_sdcard_t *sd)
{
    s_sd = sd;
}

DSTATUS disk_initialize(BYTE pdrv)
{
    if ((pdrv != 0U) || (s_sd == NULL))
    {
        return STA_NOINIT;
    }

    if (!s_sd->ready)
    {
        (void)Tm4cSdCardInit(s_sd);
    }

    return s_sd->ready ? 0U : STA_NOINIT;
}

DSTATUS disk_status(BYTE pdrv)
{
    if ((pdrv != 0U) || (s_sd == NULL) || !s_sd->ready)
    {
        return STA_NOINIT;
    }

    return 0U;
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count)
{
    if ((pdrv != 0U) || (count == 0U))
    {
        return RES_PARERR;
    }
    if ((s_sd == NULL) || !s_sd->ready)
    {
        return RES_NOTRDY;
    }

    return Tm4cSdReadSectors(s_sd, buff, (uint32_t)sector, count) ? RES_OK : RES_ERROR;
}

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count)
{
    (void)pdrv;
    (void)buff;
    (void)sector;
    (void)count;
    return RES_WRPRT;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
    uint32_t sectors;

    if (pdrv != 0U)
    {
        return RES_PARERR;
    }
    if ((s_sd == NULL) || !s_sd->ready)
    {
        return RES_NOTRDY;
    }

    switch (cmd)
    {
        case CTRL_SYNC:
            return RES_OK;

        case GET_SECTOR_COUNT:
            sectors = Tm4cSdSectorCount(s_sd);
            *(DWORD *)buff = sectors;
            return (sectors != 0U) ? RES_OK : RES_ERROR;

        case GET_SECTOR_SIZE:
            *(WORD *)buff = 512U;
            return RES_OK;

        case GET_BLOCK_SIZE:
            *(DWORD *)buff = 1U;
            return RES_OK;

        default:
            return RES_PARERR;
    }
}
