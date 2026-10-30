/*
 * tm4c_spi.h - obecný SPI master (SSI) pro TM4C129 / TivaWare.
 *
 * Žádné globální proměnné ani závislost na projektu: všechno se předává
 * strukturou tm4c_spi_t. Používá ho tm4c_sdcard (SSI1) i tm4c_lcd_st7796s (SSI0).
 *
 * Příklad (SSI1: CLK PB5, MOSI PE4, MISO PE5):
 *   static const tm4c_spi_t spi = {
 *       SYSCTL_PERIPH_SSI1, SSI1_BASE,
 *       { SYSCTL_PERIPH_GPIOB, GPIO_PORTB_BASE, GPIO_PIN_5, GPIO_PB5_SSI1CLK },
 *       { SYSCTL_PERIPH_GPIOE, GPIO_PORTE_BASE, GPIO_PIN_4, GPIO_PE4_SSI1XDAT0 },
 *       { SYSCTL_PERIPH_GPIOE, GPIO_PORTE_BASE, GPIO_PIN_5, GPIO_PE5_SSI1XDAT1 },
 *       { 0, 0, 0, 0 },                      // bez hardwarového FSS
 *       SSI_FRF_MOTO_MODE_0, 8U };
 *   Tm4cSpiInit(&spi, sysclk, 400000U);
 */
#ifndef TM4C_SPI_H
#define TM4C_SPI_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    uint32_t gpio_periph;  /* SYSCTL_PERIPH_GPIOx */
    uint32_t gpio_base;    /* GPIO_PORTx_BASE */
    uint32_t pin;          /* GPIO_PIN_n; 0 = signál se nepoužívá (jen FSS) */
    uint32_t pin_config;   /* GPIO_Pxn_SSIyCLK apod. z driverlib/pin_map.h */
} tm4c_spi_pin_t;

typedef struct
{
    uint32_t ssi_periph;   /* SYSCTL_PERIPH_SSIn */
    uint32_t ssi_base;     /* SSIn_BASE */
    tm4c_spi_pin_t clk;
    tm4c_spi_pin_t mosi;
    tm4c_spi_pin_t miso;
    tm4c_spi_pin_t fss;    /* pin == 0: CS řeší aplikace přes GPIO */
    uint32_t protocol;     /* SSI_FRF_MOTO_MODE_0..3 */
    uint32_t data_width;   /* 4..16 bitů */
} tm4c_spi_t;

/* Zapne periferie, nastaví piny a SSI; false při timeoutu zapnutí periferií. */
bool Tm4cSpiInit(const tm4c_spi_t *spi, uint32_t sysclk_hz, uint32_t bitrate_hz);

/* Změní rychlost za běhu (SSI se na chvíli vypne a vyprázdní se RX FIFO). */
void Tm4cSpiSetBitrate(const tm4c_spi_t *spi, uint32_t sysclk_hz, uint32_t bitrate_hz);

/* Pošle jedno slovo a vrátí současně přijaté. */
uint32_t Tm4cSpiTransfer(const tm4c_spi_t *spi, uint32_t value);

/* Rychlé čtení: posílá fill a plní buffer (8bitový režim), TX FIFO drží plné. */
void Tm4cSpiReadBlock(const tm4c_spi_t *spi, uint8_t *buffer, uint32_t length, uint8_t fill);

#endif
