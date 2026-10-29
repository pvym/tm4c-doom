#ifndef TM4C_SD_DRIVER_H
#define TM4C_SD_DRIVER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum
{
    SD_CARD_INIT_OK,
    SD_CARD_INIT_BUS_ERROR,
    SD_CARD_INIT_CMD0_ERROR,
    SD_CARD_INIT_CMD8_ERROR,
    SD_CARD_INIT_ACMD41_ERROR,
    SD_CARD_INIT_CMD1_ERROR,
    SD_CARD_INIT_CMD58_ERROR,
    SD_CARD_INIT_CMD16_ERROR
} sd_card_init_result_t;

bool SD_DriverBusInit(void);
sd_card_init_result_t SD_DriverCardInit(void);
uint8_t SD_DriverCmd0Response(void);
bool SD_DriverCmd0WaitReadyTimedOut(void);
bool SD_DriverMount(void);
uintptr_t SD_DriverWadCacheBase(void);
size_t SD_DriverWadCacheSize(void);
const char *SD_DriverDefaultWadPath(void);

#endif
