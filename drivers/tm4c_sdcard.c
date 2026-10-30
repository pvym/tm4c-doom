#include "tm4c_sdcard.h"

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"

#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

#define SD_CMD0   0U
#define SD_CMD1   1U
#define SD_CMD8   8U
#define SD_CMD9   9U
#define SD_CMD12  12U
#define SD_CMD16  16U
#define SD_CMD17  17U
#define SD_CMD18  18U
#define SD_CMD55  55U
#define SD_CMD58  58U
#define SD_ACMD41 41U
#define SD_APP    0x80U /* příznak: před příkazem pošli CMD55 */

#define SD_TOKEN_START_BLOCK 0xFEU

#define CT_MMC   0x01U
#define CT_SD1   0x02U
#define CT_SD2   0x04U
#define CT_BLOCK 0x08U

static void Select(tm4c_sdcard_t *sd, bool selected)
{
    GPIOPinWrite(sd->cfg->cs.gpio_base, sd->cfg->cs.pin,
                 selected ? 0U : sd->cfg->cs.pin);
}

static uint8_t Transfer(tm4c_sdcard_t *sd, uint8_t value)
{
    return (uint8_t)Tm4cSpiTransfer(&sd->cfg->spi, value);
}

static void Deselect(tm4c_sdcard_t *sd)
{
    Select(sd, false);
    Transfer(sd, 0xFFU);
}

static bool WaitReady(tm4c_sdcard_t *sd, uint32_t attempts)
{
    while (attempts-- > 0U)
    {
        if (Transfer(sd, 0xFFU) == 0xFFU)
        {
            return true;
        }
    }

    return false;
}

static uint8_t SendCommand(tm4c_sdcard_t *sd, uint8_t command, uint32_t argument)
{
    uint8_t crc = 0x01U;
    uint8_t response;

    if ((command & SD_APP) != 0U)
    {
        command &= (uint8_t)~SD_APP;
        response = SendCommand(sd, SD_CMD55, 0U);
        if (response > 1U)
        {
            return response;
        }
    }

    /* CMD12 se posílá uprostřed vícebloku, karta je zaneprázdněná. */
    if (command != SD_CMD12)
    {
        Deselect(sd);
        Select(sd, true);

        sd->cmd_wait_ready_timed_out = false;
        if (!WaitReady(sd, 50000U))
        {
            sd->cmd_wait_ready_timed_out = true;
            Deselect(sd);
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

    Transfer(sd, (uint8_t)(0x40U | command));
    Transfer(sd, (uint8_t)(argument >> 24));
    Transfer(sd, (uint8_t)(argument >> 16));
    Transfer(sd, (uint8_t)(argument >> 8));
    Transfer(sd, (uint8_t)argument);
    Transfer(sd, crc);

    if (command == SD_CMD12)
    {
        Transfer(sd, 0xFFU); /* zahodit stuff byte */
    }

    for (uint32_t i = 0U; i < 10U; ++i)
    {
        response = Transfer(sd, 0xFFU);
        if ((response & 0x80U) == 0U)
        {
            return response;
        }
    }

    return 0xFFU;
}

static bool ReceiveDataBlock(tm4c_sdcard_t *sd, uint8_t *buffer, uint32_t length)
{
    for (uint32_t i = 0U; i < 100000U; ++i)
    {
        if (Transfer(sd, 0xFFU) == SD_TOKEN_START_BLOCK)
        {
            Tm4cSpiReadBlock(&sd->cfg->spi, buffer, length, 0xFFU);
            Transfer(sd, 0xFFU); /* CRC */
            Transfer(sd, 0xFFU);
            return true;
        }
    }

    return false;
}

static bool ReadRegister(tm4c_sdcard_t *sd, uint8_t command, uint8_t *buffer, uint32_t length)
{
    bool ok = false;

    if (SendCommand(sd, command, 0U) == 0U)
    {
        ok = ReceiveDataBlock(sd, buffer, length);
    }

    Deselect(sd);
    return ok;
}

bool Tm4cSdBusInit(tm4c_sdcard_t *sd, const tm4c_sdcard_cfg_t *cfg)
{
    uint32_t timeout = 1000000U;

    sd->cfg = cfg;
    sd->card_type = 0U;
    sd->cmd0_response = 0xFFU;
    sd->bus_initialized = false;
    sd->ready = false;
    sd->cmd_wait_ready_timed_out = false;
    sd->cmd0_wait_ready_timed_out = false;
    sd->init_result = TM4C_SD_INIT_BUS_ERROR;

    SysCtlPeripheralEnable(cfg->cs.gpio_periph);
    while (!SysCtlPeripheralReady(cfg->cs.gpio_periph))
    {
        if (--timeout == 0U)
        {
            return false;
        }
    }

    GPIOPinTypeGPIOOutput(cfg->cs.gpio_base, cfg->cs.pin);
    GPIOPadConfigSet(cfg->cs.gpio_base, cfg->cs.pin,
                     GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);
    Select(sd, false);

    if (!Tm4cSpiInit(&cfg->spi, cfg->sysclk_hz, cfg->init_bitrate_hz))
    {
        return false;
    }

    sd->bus_initialized = true;
    return true;
}

tm4c_sd_init_result_t Tm4cSdCardInit(tm4c_sdcard_t *sd)
{
    uint8_t ocr[4];
    uint8_t response = 0xFFU;
    uint32_t timeout;
    bool ready = false;

    sd->ready = false;
    sd->card_type = 0U;

    if (!sd->bus_initialized)
    {
        sd->init_result = TM4C_SD_INIT_BUS_ERROR;
        return sd->init_result;
    }

    Tm4cSpiSetBitrate(&sd->cfg->spi, sd->cfg->sysclk_hz, sd->cfg->init_bitrate_hz);

    /* Min. 74 hodin s CS=1, pak CMD0 (opakovaně, karta se může probouzet). */
    for (uint32_t attempt = 0U; attempt < 20U; ++attempt)
    {
        Deselect(sd);
        for (uint32_t i = 0U; i < 10U; ++i)
        {
            Transfer(sd, 0xFFU);
        }

        response = SendCommand(sd, SD_CMD0, 0U);
        sd->cmd0_response = response;
        sd->cmd0_wait_ready_timed_out = sd->cmd_wait_ready_timed_out;
        if (response == 1U)
        {
            break;
        }

        SysCtlDelay(sd->cfg->sysclk_hz / 3000U); /* 1 ms */
    }

    if (response != 1U)
    {
        sd->init_result = TM4C_SD_INIT_CMD0_ERROR;
        Deselect(sd);
        return sd->init_result;
    }

    response = SendCommand(sd, SD_CMD8, 0x1AAU);
    if (response == 1U)
    {
        /* SD v2 (SDHC/SDXC nebo SDSC v2). */
        for (uint32_t i = 0U; i < 4U; ++i)
        {
            ocr[i] = Transfer(sd, 0xFFU);
        }

        if ((ocr[2] != 0x01U) || (ocr[3] != 0xAAU))
        {
            sd->init_result = TM4C_SD_INIT_CMD8_ERROR;
            Deselect(sd);
            return sd->init_result;
        }

        for (timeout = 100000U; timeout > 0U; --timeout)
        {
            if (SendCommand(sd, SD_APP | SD_ACMD41, 1UL << 30) == 0U)
            {
                ready = true;
                break;
            }
        }
        if (!ready)
        {
            sd->init_result = TM4C_SD_INIT_ACMD41_ERROR;
            Deselect(sd);
            return sd->init_result;
        }

        if (SendCommand(sd, SD_CMD58, 0U) != 0U)
        {
            sd->init_result = TM4C_SD_INIT_CMD58_ERROR;
            Deselect(sd);
            return sd->init_result;
        }

        for (uint32_t i = 0U; i < 4U; ++i)
        {
            ocr[i] = Transfer(sd, 0xFFU);
        }
        sd->card_type = CT_SD2 | (((ocr[0] & 0x40U) != 0U) ? CT_BLOCK : 0U);
    }
    else if (response == 0x05U)
    {
        /* SD v1 nebo MMC (CMD8 není podporován). */
        uint8_t command;

        if (SendCommand(sd, SD_APP | SD_ACMD41, 0U) <= 1U)
        {
            sd->card_type = CT_SD1;
            command = SD_APP | SD_ACMD41;
            sd->init_result = TM4C_SD_INIT_ACMD41_ERROR;
        }
        else
        {
            sd->card_type = CT_MMC;
            command = SD_CMD1;
            sd->init_result = TM4C_SD_INIT_CMD1_ERROR;
        }

        for (timeout = 100000U; timeout > 0U; --timeout)
        {
            if (SendCommand(sd, command, 0U) == 0U)
            {
                ready = true;
                break;
            }
        }
        if (!ready)
        {
            Deselect(sd);
            return sd->init_result;
        }

        if (SendCommand(sd, SD_CMD16, 512U) != 0U)
        {
            sd->card_type = 0U;
            sd->init_result = TM4C_SD_INIT_CMD16_ERROR;
            Deselect(sd);
            return sd->init_result;
        }
    }
    else
    {
        sd->init_result = TM4C_SD_INIT_CMD8_ERROR;
        Deselect(sd);
        return sd->init_result;
    }

    Deselect(sd);

    sd->ready = true;
    sd->init_result = TM4C_SD_INIT_OK;
    Tm4cSpiSetBitrate(&sd->cfg->spi, sd->cfg->sysclk_hz, sd->cfg->transfer_bitrate_hz);
    return sd->init_result;
}

bool Tm4cSdReadSectors(tm4c_sdcard_t *sd, uint8_t *buffer, uint32_t sector, uint32_t count)
{
    bool ok;

    if (!sd->ready || (count == 0U))
    {
        return false;
    }

    /* Karty bez blokového adresování (SDSC, MMC) používají bajtové adresy. */
    if ((sd->card_type & CT_BLOCK) == 0U)
    {
        sector *= 512UL;
    }

    if (count == 1U)
    {
        ok = (SendCommand(sd, SD_CMD17, sector) == 0U) &&
             ReceiveDataBlock(sd, buffer, 512U);
        Deselect(sd);
        return ok;
    }

    ok = (SendCommand(sd, SD_CMD18, sector) == 0U);
    for (uint32_t i = 0U; ok && (i < count); ++i)
    {
        ok = ReceiveDataBlock(sd, buffer + (i * 512U), 512U);
    }

    (void)SendCommand(sd, SD_CMD12, 0U);
    Deselect(sd);
    return ok;
}

uint32_t Tm4cSdSectorCount(tm4c_sdcard_t *sd)
{
    uint8_t csd[16];

    if (!sd->ready || !ReadRegister(sd, SD_CMD9, csd, sizeof(csd)))
    {
        return 0U;
    }

    if ((csd[0] & 0xC0U) == 0x40U)
    {
        uint32_t csize = ((uint32_t)(csd[7] & 0x3FU) << 16) |
                         ((uint32_t)csd[8] << 8) |
                         (uint32_t)csd[9];
        return (csize + 1UL) << 10;
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

        return (blocknr * (1UL << read_bl_len)) / 512UL;
    }
}
