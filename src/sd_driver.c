#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "config.h"
#include "sd_driver.h"

#include "diskio.h"
#include "ff.h"

#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"

#define SD_CMD0     0U
#define SD_CMD1     1U
#define SD_CMD8     8U
#define SD_CMD9     9U
#define SD_CMD12    12U
#define SD_CMD16    16U
#define SD_CMD17    17U
#define SD_CMD18    18U
#define SD_CMD55    55U
#define SD_CMD58    58U
#define SD_ACMD41   41U

#define SD_TOKEN_START_BLOCK 0xFEU

#define CT_MMC      0x01U
#define CT_SD1      0x02U
#define CT_SD2      0x04U
#define CT_BLOCK    0x08U

static FATFS s_fatfs;
static DSTATUS s_disk_status = STA_NOINIT;
static uint8_t s_card_type;
static bool s_bus_initialized;
static sd_card_init_result_t s_card_init_result = SD_CARD_INIT_BUS_ERROR;
static FRESULT s_last_fresult = FR_OK;
static uint8_t s_cmd0_response = 0xFFU;
static bool s_cmd0_wait_ready_timed_out;
static bool s_last_command_wait_ready_timed_out;

static void SD_Select(bool selected)
{
    GPIOPinWrite(TM4C_SD_CS_PORT,
                 TM4C_SD_CS_PIN,
                 selected ? 0U : TM4C_SD_CS_PIN);
}

static uint8_t SD_SPITransfer(uint8_t value)
{
    uint32_t received = 0U;

    MAP_SSIDataPut(TM4C_SD_SSI_BASE, value);
    while (MAP_SSIBusy(TM4C_SD_SSI_BASE))
    {
    }
    MAP_SSIDataGet(TM4C_SD_SSI_BASE, &received);

    return (uint8_t)received;
}

static void SD_SetSpeed(uint32_t bitrate)
{
    uint32_t discard;

    MAP_SSIDisable(TM4C_SD_SSI_BASE);
    MAP_SSIConfigSetExpClk(TM4C_SD_SSI_BASE,
                           TM4C_SYSTEM_CLOCK_HZ,
                           SSI_FRF_MOTO_MODE_0,
                           SSI_MODE_MASTER,
                           bitrate,
                           8U);
    MAP_SSIEnable(TM4C_SD_SSI_BASE);

    while (MAP_SSIBusy(TM4C_SD_SSI_BASE))
    {
    }

    while (MAP_SSIDataGetNonBlocking(TM4C_SD_SSI_BASE, &discard))
    {
    }
}

static void SD_Deselect(void)
{
    SD_Select(false);
    SD_SPITransfer(0xFFU);
}

static bool SD_WaitReady(uint32_t attempts)
{
    while (attempts-- > 0U)
    {
        if (SD_SPITransfer(0xFFU) == 0xFFU)
        {
            return true;
        }
    }

    return false;
}

static uint8_t SD_SendCommand(uint8_t command, uint32_t argument)
{
    uint8_t crc = 0x01U;
    uint8_t response;

    if ((command & 0x80U) != 0U)
    {
        command &= 0x7FU;
        response = SD_SendCommand(SD_CMD55, 0U);
        if (response > 1U)
        {
            return response;
        }
    }

    if (command != SD_CMD12)
    {
        SD_Deselect();
        SD_Select(true);

        s_last_command_wait_ready_timed_out = false;
        if (!SD_WaitReady(50000U))
        {
            s_last_command_wait_ready_timed_out = true;
            SD_Deselect();
            return 0xFFU;
        }
    }

    if (command == SD_CMD0)
    {
        crc = 0x95U;
    }
    else if (command == SD_CMD8)
    {
        crc = 0x87U;
    }

    SD_SPITransfer((uint8_t)(0x40U | command));
    SD_SPITransfer((uint8_t)(argument >> 24));
    SD_SPITransfer((uint8_t)(argument >> 16));
    SD_SPITransfer((uint8_t)(argument >> 8));
    SD_SPITransfer((uint8_t)argument);
    SD_SPITransfer(crc);

    if (command == SD_CMD12)
    {
        SD_SPITransfer(0xFFU);
    }

    for (uint32_t i = 0; i < 10U; ++i)
    {
        response = SD_SPITransfer(0xFFU);
        if ((response & 0x80U) == 0U)
        {
            return response;
        }
    }

    return 0xFFU;
}

// Plní TX FIFO předem (8 položek), takže SSI běží bez mezer mezi bajty.
static void SD_ReceiveBytes(uint8_t *buffer, uint32_t length)
{
    uint32_t sent = (length < 8U) ? length : 8U;
    uint32_t received;

    for (uint32_t i = 0U; i < sent; ++i)
    {
        MAP_SSIDataPut(TM4C_SD_SSI_BASE, 0xFFU);
    }

    for (uint32_t i = 0U; i < length; ++i)
    {
        MAP_SSIDataGet(TM4C_SD_SSI_BASE, &received);
        buffer[i] = (uint8_t)received;

        if (sent < length)
        {
            MAP_SSIDataPut(TM4C_SD_SSI_BASE, 0xFFU);
            ++sent;
        }
    }
}

static bool SD_ReceiveDataBlock(uint8_t *buffer, uint32_t length)
{
    uint8_t token;

    for (uint32_t i = 0; i < 100000U; ++i)
    {
        token = SD_SPITransfer(0xFFU);
        if (token == SD_TOKEN_START_BLOCK)
        {
            SD_ReceiveBytes(buffer, length);

            SD_SPITransfer(0xFFU);
            SD_SPITransfer(0xFFU);
            return true;
        }
    }

    return false;
}

static bool SD_ReadRegister(uint8_t command, uint8_t *buffer, uint32_t length)
{
    bool ok = false;

    if (SD_SendCommand(command, 0U) == 0U)
    {
        ok = SD_ReceiveDataBlock(buffer, length);
    }

    SD_Deselect();
    return ok;
}

static DWORD SD_SectorCount(void)
{
    uint8_t csd[16];
    DWORD sectors = 0U;

    if (!SD_ReadRegister(SD_CMD9, csd, sizeof(csd)))
    {
        return 0U;
    }

    if ((csd[0] & 0xC0U) == 0x40U)
    {
        uint32_t csize = ((uint32_t)(csd[7] & 0x3FU) << 16) |
                         ((uint32_t)csd[8] << 8) |
                         (uint32_t)csd[9];
        sectors = (DWORD)((csize + 1UL) << 10);
    }
    else
    {
        uint32_t read_bl_len = csd[5] & 0x0FU;
        uint32_t c_size = ((uint32_t)(csd[6] & 0x03U) << 10) |
                          ((uint32_t)csd[7] << 2) |
                          ((csd[8] & 0xC0U) >> 6);
        uint32_t c_size_mult = ((uint32_t)(csd[9] & 0x03U) << 1) |
                               ((csd[10] & 0x80U) >> 7);
        uint32_t blocknr = (c_size + 1UL) << (c_size_mult + 2UL);
        uint32_t block_len = 1UL << read_bl_len;
        sectors = (DWORD)((blocknr * block_len) / 512UL);
    }

    return sectors;
}

static bool SD_BusInit(void)
{
    if (s_bus_initialized)
    {
        return true;
    }

    SysCtlPeripheralEnable(TM4C_SD_GPIOB_PERIPH);
    SysCtlPeripheralEnable(TM4C_SD_GPIOE_PERIPH);
    SysCtlPeripheralEnable(TM4C_SD_SSI_PERIPH);

    for (uint32_t timeout = 1000000U; timeout > 0U; --timeout)
    {
        if (SysCtlPeripheralReady(TM4C_SD_GPIOB_PERIPH) &&
            SysCtlPeripheralReady(TM4C_SD_GPIOE_PERIPH) &&
            SysCtlPeripheralReady(TM4C_SD_SSI_PERIPH))
        {
            break;
        }

        if (timeout == 1U)
        {
            return false;
        }
    }

    SysCtlPeripheralEnable(TM4C_SD_CS_PERIPH);
    if (!SysCtlPeripheralReady(TM4C_SD_CS_PERIPH))
    {
        return false;
    }

    GPIOPinConfigure(GPIO_PB5_SSI1CLK);
    GPIOPinConfigure(GPIO_PE4_SSI1XDAT0);
    GPIOPinConfigure(GPIO_PE5_SSI1XDAT1);

    GPIOPinTypeSSI(TM4C_SD_CLK_PORT, TM4C_SD_CLK_PIN);
    GPIOPinTypeSSI(TM4C_SD_MOSI_PORT, TM4C_SD_MOSI_PIN);
    GPIOPadConfigSet(TM4C_SD_CLK_PORT,
                     TM4C_SD_CLK_PIN,
                     GPIO_STRENGTH_8MA,
                     GPIO_PIN_TYPE_STD);
    GPIOPadConfigSet(TM4C_SD_MOSI_PORT,
                     TM4C_SD_MOSI_PIN,
                     GPIO_STRENGTH_8MA,
                     GPIO_PIN_TYPE_STD);
    GPIOPinTypeSSI(TM4C_SD_MISO_PORT, TM4C_SD_MISO_PIN);
    GPIOPadConfigSet(TM4C_SD_MISO_PORT,
                     TM4C_SD_MISO_PIN,
                     GPIO_STRENGTH_2MA,
                     GPIO_PIN_TYPE_STD_WPU);

    GPIOPinTypeGPIOOutput(TM4C_SD_CS_PORT, TM4C_SD_CS_PIN);
    GPIOPadConfigSet(TM4C_SD_CS_PORT,
                     TM4C_SD_CS_PIN,
                     GPIO_STRENGTH_2MA,
                     GPIO_PIN_TYPE_STD);

    SD_Select(false);
    SD_SetSpeed(TM4C_SD_INIT_HZ);
    s_bus_initialized = true;
    return true;
}

DSTATUS disk_initialize(BYTE pdrv)
{
    uint8_t ocr[4];
    uint8_t response;
    uint32_t timeout;
    bool ready = false;

    if (pdrv != 0U)
    {
        return STA_NOINIT;
    }

    if (s_disk_status == 0U)
    {
        s_card_init_result = SD_CARD_INIT_OK;
        return s_disk_status;
    }

    if (!SD_BusInit())
    {
        s_card_init_result = SD_CARD_INIT_BUS_ERROR;
        s_disk_status = STA_NOINIT;
        return s_disk_status;
    }

    s_card_type = 0U;

    for (uint32_t attempt = 0U; attempt < 20U; ++attempt)
    {
        SD_Deselect();
        for (uint32_t i = 0; i < 10U; ++i)
        {
            SD_SPITransfer(0xFFU);
        }

        response = SD_SendCommand(SD_CMD0, 0U);
        s_cmd0_response = response;
        s_cmd0_wait_ready_timed_out = s_last_command_wait_ready_timed_out;
        if (response == 1U)
        {
            break;
        }

        SysCtlDelay(TM4C_SYSTEM_CLOCK_HZ / 3000U);
    }

    if (response != 1U)
    {
        s_card_init_result = SD_CARD_INIT_CMD0_ERROR;
        SD_Deselect();
        return STA_NOINIT;
    }

    response = SD_SendCommand(SD_CMD8, 0x1AAU);
    if (response == 1U)
    {
        for (uint32_t i = 0; i < 4U; ++i)
        {
            ocr[i] = SD_SPITransfer(0xFFU);
        }

        if ((ocr[2] != 0x01U) || (ocr[3] != 0xAAU))
        {
            s_card_init_result = SD_CARD_INIT_CMD8_ERROR;
            SD_Deselect();
            return STA_NOINIT;
        }

        s_card_init_result = SD_CARD_INIT_ACMD41_ERROR;
        timeout = 100000U;
        while (timeout > 0U)
        {
            --timeout;
            if (SD_SendCommand(0x80U | SD_ACMD41, 1UL << 30) == 0U)
            {
                ready = true;
                break;
            }
        }

        if (!ready)
        {
            SD_Deselect();
            return STA_NOINIT;
        }

        s_card_init_result = SD_CARD_INIT_CMD58_ERROR;
        if (SD_SendCommand(SD_CMD58, 0U) != 0U)
        {
            SD_Deselect();
            return STA_NOINIT;
        }

        for (uint32_t i = 0; i < 4U; ++i)
        {
            ocr[i] = SD_SPITransfer(0xFFU);
        }
        s_card_type = CT_SD2 | ((ocr[0] & 0x40U) ? CT_BLOCK : 0U);
    }
    else if (response == 0x05U)
    {
        response = SD_SendCommand(0x80U | SD_ACMD41, 0U);
        if (response <= 1U)
        {
            s_card_type = CT_SD1;
            s_card_init_result = SD_CARD_INIT_ACMD41_ERROR;
            timeout = 100000U;
            ready = false;
            while (timeout > 0U)
            {
                --timeout;
                if (SD_SendCommand(0x80U | SD_ACMD41, 0U) == 0U)
                {
                    ready = true;
                    break;
                }
            }

            if (!ready)
            {
                SD_Deselect();
                return STA_NOINIT;
            }
        }
        else
        {
            s_card_type = CT_MMC;
            s_card_init_result = SD_CARD_INIT_CMD1_ERROR;
            timeout = 100000U;
            ready = false;
            while (timeout > 0U)
            {
                --timeout;
                if (SD_SendCommand(SD_CMD1, 0U) == 0U)
                {
                    ready = true;
                    break;
                }
            }

            if (!ready)
            {
                SD_Deselect();
                return STA_NOINIT;
            }
        }

        s_card_init_result = SD_CARD_INIT_CMD16_ERROR;
        if (SD_SendCommand(SD_CMD16, 512U) != 0U)
        {
            s_card_type = 0U;
            SD_Deselect();
            return STA_NOINIT;
        }
    }
    else
    {
        s_card_init_result = SD_CARD_INIT_CMD8_ERROR;
        SD_Deselect();
        return STA_NOINIT;
    }

    SD_Deselect();

    if (s_card_type != 0U)
    {
        s_disk_status = 0U;
        s_card_init_result = SD_CARD_INIT_OK;
        SD_SetSpeed(TM4C_SD_TRANSFER_HZ);
    }
    else
    {
        s_card_init_result = SD_CARD_INIT_CMD8_ERROR;
        s_disk_status = STA_NOINIT;
    }

    return s_disk_status;
}

DSTATUS disk_status(BYTE pdrv)
{
    return (pdrv == 0U) ? s_disk_status : STA_NOINIT;
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count)
{
    if ((pdrv != 0U) || (count == 0U))
    {
        return RES_PARERR;
    }

    if ((s_disk_status & STA_NOINIT) != 0U)
    {
        return RES_NOTRDY;
    }

    if ((s_card_type & CT_BLOCK) == 0U)
    {
        sector *= 512UL;
    }

    if (count > 1U)
    {
        bool ok = (SD_SendCommand(SD_CMD18, (uint32_t)sector) == 0U);

        for (UINT i = 0; ok && (i < count); ++i)
        {
            ok = SD_ReceiveDataBlock(buff + (i * 512U), 512U);
        }

        (void)SD_SendCommand(SD_CMD12, 0U);
        SD_Deselect();
        return ok ? RES_OK : RES_ERROR;
    }

    if (SD_SendCommand(SD_CMD17, (uint32_t)sector) != 0U)
    {
        SD_Deselect();
        return RES_ERROR;
    }

    if (!SD_ReceiveDataBlock(buff, 512U))
    {
        SD_Deselect();
        return RES_ERROR;
    }

    SD_Deselect();
    return RES_OK;
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
    if (pdrv != 0U)
    {
        return RES_PARERR;
    }

    if ((s_disk_status & STA_NOINIT) != 0U)
    {
        return RES_NOTRDY;
    }

    switch (cmd)
    {
        case CTRL_SYNC:
            return RES_OK;

        case GET_SECTOR_COUNT:
            *(DWORD *)buff = SD_SectorCount();
            return (*(DWORD *)buff != 0U) ? RES_OK : RES_ERROR;

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

bool SD_DriverBusInit(void)
{
    return SD_BusInit();
}

sd_card_init_result_t SD_DriverCardInit(void)
{
    (void)disk_initialize(0U);
    return s_card_init_result;
}

uint8_t SD_DriverCmd0Response(void)
{
    return s_cmd0_response;
}

bool SD_DriverCmd0WaitReadyTimedOut(void)
{
    return s_cmd0_wait_ready_timed_out;
}

bool SD_DriverMount(void)
{
    s_last_fresult = f_mount(&s_fatfs, "0:", 1U);
    return s_last_fresult == FR_OK;
}

uint8_t SD_DriverLastFatFsResult(void)
{
    return (uint8_t)s_last_fresult;
}

sd_wad_check_result_t SD_DriverWadCheck(const char *path)
{
    FIL file;
    uint8_t header[12];
    UINT bytes_read = 0U;
    volatile uint8_t *cache = (volatile uint8_t *)TM4C_WAD_CACHE_ADDR;
    FSIZE_t size;
    FSIZE_t offset;
    sd_wad_check_result_t result = SD_WAD_CHECK_OK;

    s_last_fresult = f_open(&file, path, FA_READ | FA_OPEN_EXISTING);
    if (s_last_fresult != FR_OK)
    {
        return SD_WAD_CHECK_OPEN_ERROR;
    }

    size = f_size(&file);
    if (size < sizeof(header))
    {
        result = SD_WAD_CHECK_TOO_SMALL;
    }
    else if (size > TM4C_WAD_CACHE_SIZE)
    {
        result = SD_WAD_CHECK_TOO_BIG;
    }
    else
    {
        s_last_fresult = f_read(&file, header, sizeof(header), &bytes_read);
        if ((s_last_fresult != FR_OK) || (bytes_read != sizeof(header)))
        {
            result = SD_WAD_CHECK_HEADER_READ_ERROR;
        }
        else if ((header[1] != 'W') || (header[2] != 'A') || (header[3] != 'D') ||
                 ((header[0] != 'I') && (header[0] != 'P')))
        {
            result = SD_WAD_CHECK_BAD_MAGIC;
        }
    }

    if (result == SD_WAD_CHECK_OK)
    {
        s_last_fresult = f_lseek(&file, 0U);
        for (offset = 0U; (s_last_fresult == FR_OK) && (offset < size); offset += bytes_read)
        {
            UINT chunk = ((size - offset) > 4096U) ? 4096U : (UINT)(size - offset);

            s_last_fresult = f_read(&file, (void *)(cache + offset), chunk, &bytes_read);
            if ((s_last_fresult == FR_OK) && (bytes_read != chunk))
            {
                s_last_fresult = FR_INT_ERR;
            }
        }

        if (s_last_fresult != FR_OK)
        {
            result = SD_WAD_CHECK_CACHE_READ_ERROR;
        }
        else if ((cache[1] != 'W') || (cache[2] != 'A') || (cache[3] != 'D'))
        {
            result = SD_WAD_CHECK_CACHE_VERIFY_ERROR;
        }
    }

    (void)f_close(&file);
    return result;
}

uintptr_t SD_DriverWadCacheBase(void)
{
    return (uintptr_t)TM4C_WAD_CACHE_ADDR;
}

size_t SD_DriverWadCacheSize(void)
{
    return (size_t)TM4C_WAD_CACHE_SIZE;
}

const char *SD_DriverDefaultWadPath(void)
{
    return TM4C_WAD_PATH;
}

DWORD get_fattime(void)
{
    return ((DWORD)(2024U - 1980U) << 25) |
           ((DWORD)1U << 21) |
           ((DWORD)1U << 16);
}
