/*
 * tm4c_sdcard.h - microSD karta v SPI režimu přes SSI (TM4C129 / TivaWare).
 *
 * Jen čtení (CMD17/CMD18), SDSC/SDHC/SDXC a MMC. Bez závislosti na FatFs;
 * propojení s FatFs je v tm4c_sdcard_diskio.c.
 *
 * Příklad:
 *   static tm4c_sdcard_t sd;
 *   static const tm4c_sdcard_cfg_t cfg = {
 *       { SYSCTL_PERIPH_SSI1, SSI1_BASE, ... , SSI_FRF_MOTO_MODE_0, 8U },
 *       { SYSCTL_PERIPH_GPIOB, GPIO_PORTB_BASE, GPIO_PIN_4, 0 },   // CS
 *       120000000UL, 400000UL, 20000000UL };
 *   Tm4cSdBusInit(&sd, &cfg);
 *   if (Tm4cSdCardInit(&sd) == TM4C_SD_INIT_OK) {
 *       Tm4cSdReadSectors(&sd, buffer, 0U, 1U);
 *   }
 */
#ifndef TM4C_SDCARD_H
#define TM4C_SDCARD_H

#include <stdbool.h>
#include <stdint.h>

#include "tm4c_spi.h"

typedef enum
{
    TM4C_SD_INIT_OK,
    TM4C_SD_INIT_BUS_ERROR,
    TM4C_SD_INIT_CMD0_ERROR,
    TM4C_SD_INIT_CMD8_ERROR,
    TM4C_SD_INIT_ACMD41_ERROR,
    TM4C_SD_INIT_CMD1_ERROR,
    TM4C_SD_INIT_CMD58_ERROR,
    TM4C_SD_INIT_CMD16_ERROR
} tm4c_sd_init_result_t;

typedef struct
{
    tm4c_spi_t spi;           /* 8bitový SPI mód 0, CS se neřeší hardwarově */
    tm4c_spi_pin_t cs;        /* GPIO chip select (pin_config se nepoužívá) */
    uint32_t sysclk_hz;
    uint32_t init_bitrate_hz; /* při inicializaci karty, max 400 kHz */
    uint32_t transfer_bitrate_hz; /* po inicializaci, typicky 12-25 MHz */
} tm4c_sdcard_cfg_t;

typedef struct
{
    const tm4c_sdcard_cfg_t *cfg;
    uint8_t card_type;
    uint8_t cmd0_response;
    bool bus_initialized;
    bool ready;
    bool cmd_wait_ready_timed_out;
    bool cmd0_wait_ready_timed_out;
    tm4c_sd_init_result_t init_result;
} tm4c_sdcard_t;

/* Nastaví piny, SSI a CS (SPI na init_bitrate_hz). */
bool Tm4cSdBusInit(tm4c_sdcard_t *sd, const tm4c_sdcard_cfg_t *cfg);

/* Inicializace karty (CMD0, CMD8, ACMD41, CMD58, CMD16), pak přepne na rychlý přenos. */
tm4c_sd_init_result_t Tm4cSdCardInit(tm4c_sdcard_t *sd);

/* Přečte count sektorů po 512 B (count > 1 použije CMD18). */
bool Tm4cSdReadSectors(tm4c_sdcard_t *sd, uint8_t *buffer, uint32_t sector, uint32_t count);

/* Počet 512B sektorů z CSD, 0 při chybě. */
uint32_t Tm4cSdSectorCount(tm4c_sdcard_t *sd);

#endif
