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

    SD_Deselect();
    SD_Select(true);

    if (!SD_WaitReady(50000U))
    {
        SD_Deselect();
        return 0xFFU;
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

static bool SD_ReceiveDataBlock(uint8_t *buffer, uint32_t length)
{
    uint8_t token;

    for (uint32_t i = 0; i < 100000U; ++i)
    {
        token = SD_SPITransfer(0xFFU);
        if (token == SD_TOKEN_START_BLOCK)
        {
            for (uint32_t j = 0; j < length; ++j)
            {
                buffer[j] = SD_SPITransfer(0xFFU);
            }

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

static void SD_BusInit(void)
{
    SysCtlPeripheralEnable(TM4C_SD_GPIOB_PERIPH);
    while (!SysCtlPeripheralReady(TM4C_SD_GPIOB_PERIPH))
    {
    }

    SysCtlPeripheralEnable(TM4C_SD_GPIOE_PERIPH);
    while (!SysCtlPeripheralReady(TM4C_SD_GPIOE_PERIPH))
    {
    }

    SysCtlPeripheralEnable(TM4C_SD_CS_PERIPH);
    while (!SysCtlPeripheralReady(TM4C_SD_CS_PERIPH))
    {
    }

    SysCtlPeripheralEnable(TM4C_SD_SSI_PERIPH);
    while (!SysCtlPeripheralReady(TM4C_SD_SSI_PERIPH))
    {
    }

    GPIOPinConfigure(GPIO_PB5_SSI1CLK);
    GPIOPinConfigure(GPIO_PE4_SSI1XDAT0);
    GPIOPinConfigure(GPIO_PE5_SSI1XDAT1);

    GPIOPinTypeSSI(TM4C_SD_CLK_PORT, TM4C_SD_CLK_PIN);
    GPIOPinTypeSSI(TM4C_SD_MOSI_PORT, TM4C_SD_MOSI_PIN);
    GPIOPinTypeSSI(TM4C_SD_MISO_PORT, TM4C_SD_MISO_PIN);

    GPIOPinTypeGPIOOutput(TM4C_SD_CS_PORT, TM4C_SD_CS_PIN);
    GPIOPadConfigSet(TM4C_SD_CS_PORT,
                     TM4C_SD_CS_PIN,
                     GPIO_STRENGTH_2MA,
                     GPIO_PIN_TYPE_STD);

    SD_Select(false);
    SD_SetSpeed(TM4C_SD_INIT_HZ);
}

DSTATUS disk_initialize(BYTE pdrv)
{
    uint8_t ocr[4];
    uint32_t timeout;

    if (pdrv != 0U)
    {
        return STA_NOINIT;
    }

    SD_BusInit();

    for (uint32_t i = 0; i < 10U; ++i)
    {
        SD_SPITransfer(0xFFU);
    }

    s_card_type = 0U;

    if (SD_SendCommand(SD_CMD0, 0U) == 1U)
    {
        if (SD_SendCommand(SD_CMD8, 0x1AAU) == 1U)
        {
            for (uint32_t i = 0; i < 4U; ++i)
            {
                ocr[i] = SD_SPITransfer(0xFFU);
            }

            if ((ocr[2] == 0x01U) && (ocr[3] == 0xAAU))
            {
                timeout = 100000U;
                while ((timeout-- > 0U) && (SD_SendCommand(0x80U | SD_ACMD41, 1UL << 30) != 0U))
                {
                }

                if ((timeout > 0U) && (SD_SendCommand(SD_CMD58, 0U) == 0U))
                {
                    for (uint32_t i = 0; i < 4U; ++i)
                    {
                        ocr[i] = SD_SPITransfer(0xFFU);
                    }
                    s_card_type = CT_SD2 | ((ocr[0] & 0x40U) ? CT_BLOCK : 0U);
                }
            }
        }
        else
        {
            uint8_t init_command = (SD_SendCommand(0x80U | SD_ACMD41, 0U) <= 1U) ? (0x80U | SD_ACMD41) : SD_CMD1;
            s_card_type = (init_command == (0x80U | SD_ACMD41)) ? CT_SD1 : CT_MMC;

            timeout = 100000U;
            while ((timeout-- > 0U) && (SD_SendCommand(init_command, 0U) != 0U))
            {
            }

            if ((timeout == 0U) || (SD_SendCommand(SD_CMD16, 512U) != 0U))
            {
                s_card_type = 0U;
            }
        }
    }

    SD_Deselect();

    if (s_card_type != 0U)
    {
        s_disk_status = 0U;
        SD_SetSpeed(TM4C_SD_TRANSFER_HZ);
    }
    else
    {
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

    for (UINT i = 0; i < count; ++i)
    {
        if (SD_SendCommand(SD_CMD17, (uint32_t)sector) != 0U)
        {
            SD_Deselect();
            return RES_ERROR;
        }

        if (!SD_ReceiveDataBlock(buff + (i * 512U), 512U))
        {
            SD_Deselect();
            return RES_ERROR;
        }

        SD_Deselect();
        sector += ((s_card_type & CT_BLOCK) != 0U) ? 1UL : 512UL;
    }

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

bool SD_DriverInit(void)
{
    return disk_initialize(0U) == 0U;
}

bool SD_DriverMount(void)
{
    return f_mount(&s_fatfs, "0:", 1U) == FR_OK;
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
