/*
 * tm4c_lcd_st7796s.h - LCD (RGB565 raster přes LCD0 + EPI SDRAM framebuffer)
 * s řadičem ST7796S konfigurovaným po SPI (TM4C129 / TivaWare).
 *
 * Zahrnuje:
 *  - nastavení pinů a rastru LCD0 (16bit RGB565, pixel clock, časování, DMA),
 *  - inicializaci panelu ST7796S příkazy přes SSI (9bitový SPI),
 *  - double buffering se synchronizací na konec snímku a počítadla chyb.
 *
 * Framebuffer musí ležet v paměti dostupné LCD DMA (typicky SDRAM,
 * viz tm4c_sdram.h - nejdřív Tm4cSdramInit()). Pozn.: LCDIntRegister() vyžaduje
 * vektorovou tabulku v RAM (startup s g_pfnRAMVectors), jinak nasměrujte
 * INT_LCD0 ve startupu na Tm4cLcdIntHandler().
 *
 * Příklad: viz komentář u tm4c_lcd_cfg_t.
 */
#ifndef TM4C_LCD_ST7796S_H
#define TM4C_LCD_ST7796S_H

#include <stdbool.h>
#include <stdint.h>

#include "tm4c_spi.h"

typedef struct
{
    uint32_t gpio_periph;
    uint32_t gpio_base;
    uint32_t pin;
} tm4c_gpio_t;

/*
 * Příklad konfigurace (320x480 rastr, MADCTL 0x08, 4 MHz pixel clock):
 *   static const tm4c_lcd_cfg_t lcd = {
 *       120000000UL, 320U, 480U, 4000000UL,
 *       10U, 3U, 3U, 3U, 3U, 3U,                 // hfp hbp hpw vfp vbp vpw
 *       { 0x60000000UL, 0x600C0000UL },          // framebuffery
 *       { SYSCTL_PERIPH_SSI0, SSI0_BASE,
 *         { SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, GPIO_PIN_2, GPIO_PA2_SSI0CLK },
 *         { SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, GPIO_PIN_4, GPIO_PA4_SSI0XDAT0 },
 *         { SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, GPIO_PIN_5, GPIO_PA5_SSI0XDAT1 },
 *         { SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, GPIO_PIN_3, GPIO_PA3_SSI0FSS },
 *         SSI_FRF_MOTO_MODE_0, 9U },
 *       1000000UL,                                // SPI panelu
 *       { SYSCTL_PERIPH_GPIOF, GPIO_PORTF_BASE, GPIO_PIN_6 },   // reset
 *       { SYSCTL_PERIPH_GPIOH, GPIO_PORTH_BASE, GPIO_PIN_5 },   // podsvit
 *       { SYSCTL_PERIPH_GPIOF, GPIO_PORTF_BASE, GPIO_PIN_4 },   // DRDX (pin 0 = není)
 *       0x08U };                                  // MADCTL
 *   Tm4cLcdInit(&lcd);
 *   uint16_t *fb = (uint16_t *)Tm4cLcdBackBuffer();   // kreslit sem
 *   Tm4cLcdSwap();                                    // zobrazit
 */
typedef struct
{
    uint32_t sysclk_hz;
    uint16_t width;            /* fyzický rastr (sloupců) */
    uint16_t height;           /* fyzický rastr (řádků) */
    uint32_t pixel_clock_hz;
    uint8_t h_front_porch;
    uint8_t h_back_porch;
    uint8_t h_pulse_width;
    uint8_t v_front_porch;
    uint8_t v_back_porch;
    uint8_t v_pulse_width;
    uintptr_t framebuffer[2];  /* framebuffer[1] == 0: bez double bufferingu */

    tm4c_spi_t panel_spi;      /* 9bitový SPI mód 0 s hardwarovým FSS */
    uint32_t panel_spi_hz;
    tm4c_gpio_t reset;
    tm4c_gpio_t backlight;
    tm4c_gpio_t drdx;          /* pin == 0: nepoužito; jinak se drží na 1 */
    uint8_t madctl;            /* registr 0x36 */
} tm4c_lcd_cfg_t;

/* Spustí raster LCD0 a inicializuje panel (trvá ~1,2 s kvůli resetu panelu). */
bool Tm4cLcdInit(const tm4c_lcd_cfg_t *cfg);

void Tm4cLcdBacklight(bool on);

/* Buffer, do kterého lze kreslit (nezobrazený). Při 1 bufferu totéž co front. */
volatile uint16_t *Tm4cLcdBackBuffer(void);
volatile uint16_t *Tm4cLcdFrontBuffer(void);

/* Zobrazí back buffer od příštího snímku a počká na přepnutí. */
void Tm4cLcdSwap(void);

/* Vyplní jeden buffer (back) barvou RGB565. */
void Tm4cLcdClear(uint16_t color);

/* Obsluha přerušení LCD0 (registruje se v Tm4cLcdInit). */
void Tm4cLcdIntHandler(void);

/* Diagnostika. */
uint32_t Tm4cLcdUnderflowCount(void);
uint32_t Tm4cLcdSyncLostCount(void);
uint32_t Tm4cLcdFrameCount(void);

#endif
