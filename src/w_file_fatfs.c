#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "ff.h"
#include "sd_driver.h"

#include "w_file.h"
#include "z_zone.h"

typedef struct
{
    wad_file_t wad;
    FIL file;
    uint8_t *cache;
    size_t cache_len;
} fatfs_wad_file_t;

static fatfs_wad_file_t *s_cached_wad;

extern wad_file_class_t fatfs_wad_file;

static wad_file_t *W_FatFs_OpenFile(char *path)
{
    fatfs_wad_file_t *result;
    FRESULT status;

    if (!SD_DriverMount())
    {
        return NULL;
    }

    result = Z_Malloc(sizeof(*result), PU_STATIC, 0);
    memset(result, 0, sizeof(*result));

    status = f_open(&result->file, path, FA_READ | FA_OPEN_EXISTING);
    if (status != FR_OK)
    {
        Z_Free(result);
        return NULL;
    }

    result->wad.file_class = &fatfs_wad_file;
    result->wad.length = (unsigned int)f_size(&result->file);

    if ((s_cached_wad == NULL) && (result->wad.length <= SD_DriverWadCacheSize()))
    {
        UINT bytes_read = 0U;
        result->cache = (uint8_t *)SD_DriverWadCacheBase();
        result->cache_len = result->wad.length;

        if ((f_lseek(&result->file, 0U) == FR_OK) &&
            (f_read(&result->file, result->cache, result->cache_len, &bytes_read) == FR_OK) &&
            (bytes_read == result->cache_len))
        {
            result->wad.mapped = result->cache;
            s_cached_wad = result;
            (void)f_lseek(&result->file, 0U);
        }
        else
        {
            result->cache = NULL;
            result->cache_len = 0U;
            result->wad.mapped = NULL;
        }
    }

    return &result->wad;
}

static void W_FatFs_CloseFile(wad_file_t *wad)
{
    fatfs_wad_file_t *fatfs_wad = (fatfs_wad_file_t *)wad;

    if (s_cached_wad == fatfs_wad)
    {
        s_cached_wad = NULL;
    }

    (void)f_close(&fatfs_wad->file);
    Z_Free(fatfs_wad);
}

static size_t W_FatFs_Read(wad_file_t *wad, unsigned int offset, void *buffer, size_t buffer_len)
{
    fatfs_wad_file_t *fatfs_wad = (fatfs_wad_file_t *)wad;
    UINT bytes_read = 0U;

    if (offset >= wad->length)
    {
        return 0U;
    }

    if ((offset + buffer_len) > wad->length)
    {
        buffer_len = wad->length - offset;
    }

    if (wad->mapped != NULL)
    {
        memcpy(buffer, wad->mapped + offset, buffer_len);
        return buffer_len;
    }

    if (f_lseek(&fatfs_wad->file, offset) != FR_OK)
    {
        return 0U;
    }

    if (f_read(&fatfs_wad->file, buffer, buffer_len, &bytes_read) != FR_OK)
    {
        return 0U;
    }

    return (size_t)bytes_read;
}

wad_file_class_t fatfs_wad_file =
{
    W_FatFs_OpenFile,
    W_FatFs_CloseFile,
    W_FatFs_Read,
};
