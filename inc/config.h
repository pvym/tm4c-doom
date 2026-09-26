#ifndef TM4C_DOOM_CONFIG_H
#define TM4C_DOOM_CONFIG_H

#include <stdint.h>

#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/lcd.h"
#include "driverlib/sysctl.h"
#include "driverlib/ssi.h"

#define TM4C_SYSTEM_CLOCK_HZ        120000000UL

#define TM4C_LCD_WIDTH              480U
#define TM4C_LCD_HEIGHT             320U
#define TM4C_DOOM_WIDTH             320U
#define TM4C_DOOM_HEIGHT            200U

#define TM4C_LCD_FRAMEBUFFER_ADDR   0x10000000UL
#define TM4C_LCD_FRAMEBUFFER_SIZE   (TM4C_LCD_WIDTH * TM4C_LCD_HEIGHT * sizeof(uint16_t))
#define TM4C_LCD_PIXEL_CLOCK_HZ     10000000UL
#define TM4C_LCD_RASTER_FLAGS       (RASTER_TIMING_ACTIVE_LOW_PIXCLK | \
                                     RASTER_TIMING_SYNCS_ON_RISING_PIXCLK | \
                                     RASTER_TIMING_ACTIVE_LOW_HSYNC | \
                                     RASTER_TIMING_ACTIVE_LOW_VSYNC | \
                                     RASTER_TIMING_ACTIVE_HIGH_OE)
#define TM4C_LCD_H_FRONT_PORCH      10U
#define TM4C_LCD_H_BACK_PORCH       3U
#define TM4C_LCD_H_PULSE_WIDTH      3U
#define TM4C_LCD_V_FRONT_PORCH      3U
#define TM4C_LCD_V_BACK_PORCH       3U
#define TM4C_LCD_V_PULSE_WIDTH      3U
#define TM4C_LCD_AC_BIAS            0U

#define TM4C_SDRAM_BASE             0x60000000UL
#define TM4C_SDRAM_SIZE             (32UL * 1024UL * 1024UL)
#define TM4C_DOOM_ZONE_HEAP_ADDR    TM4C_SDRAM_BASE
#define TM4C_DOOM_ZONE_HEAP_SIZE    (16UL * 1024UL * 1024UL)
#define TM4C_WAD_CACHE_ADDR         (TM4C_DOOM_ZONE_HEAP_ADDR + TM4C_DOOM_ZONE_HEAP_SIZE)
#define TM4C_WAD_CACHE_SIZE         (8UL * 1024UL * 1024UL)

#define TM4C_WAD_PATH               "0:/doom1.wad"

#define TM4C_INPUT_ACTIVE_LEVEL     0U

#define TM4C_BTN_UP_PERIPH          SYSCTL_PERIPH_GPIOA
#define TM4C_BTN_UP_PORT            GPIO_PORTA_BASE
#define TM4C_BTN_UP_PIN             GPIO_PIN_2

#define TM4C_BTN_DOWN_PERIPH        SYSCTL_PERIPH_GPIOA
#define TM4C_BTN_DOWN_PORT          GPIO_PORTA_BASE
#define TM4C_BTN_DOWN_PIN           GPIO_PIN_3

#define TM4C_BTN_LEFT_PERIPH        SYSCTL_PERIPH_GPIOA
#define TM4C_BTN_LEFT_PORT          GPIO_PORTA_BASE
#define TM4C_BTN_LEFT_PIN           GPIO_PIN_4

#define TM4C_BTN_RIGHT_PERIPH       SYSCTL_PERIPH_GPIOA
#define TM4C_BTN_RIGHT_PORT         GPIO_PORTA_BASE
#define TM4C_BTN_RIGHT_PIN          GPIO_PIN_5

#define TM4C_BTN_FIRE_PERIPH        SYSCTL_PERIPH_GPIOB
#define TM4C_BTN_FIRE_PORT          GPIO_PORTB_BASE
#define TM4C_BTN_FIRE_PIN           GPIO_PIN_4

#define TM4C_BTN_USE_PERIPH         SYSCTL_PERIPH_GPIOC
#define TM4C_BTN_USE_PORT           GPIO_PORTC_BASE
#define TM4C_BTN_USE_PIN            GPIO_PIN_4

#define TM4C_BTN_MENU_PERIPH        SYSCTL_PERIPH_GPIOC
#define TM4C_BTN_MENU_PORT          GPIO_PORTC_BASE
#define TM4C_BTN_MENU_PIN           GPIO_PIN_5

#define TM4C_SD_SSI_PERIPH          SYSCTL_PERIPH_SSI1
#define TM4C_SD_SSI_BASE            SSI1_BASE
#define TM4C_SD_GPIOB_PERIPH        SYSCTL_PERIPH_GPIOB
#define TM4C_SD_GPIOE_PERIPH        SYSCTL_PERIPH_GPIOE
#define TM4C_SD_CLK_PORT            GPIO_PORTB_BASE
#define TM4C_SD_CLK_PIN             GPIO_PIN_5
#define TM4C_SD_MOSI_PORT           GPIO_PORTE_BASE
#define TM4C_SD_MOSI_PIN            GPIO_PIN_4
#define TM4C_SD_MISO_PORT           GPIO_PORTE_BASE
#define TM4C_SD_MISO_PIN            GPIO_PIN_5
#define TM4C_SD_CS_PERIPH           SYSCTL_PERIPH_GPIOB
#define TM4C_SD_CS_PORT             GPIO_PORTB_BASE
#define TM4C_SD_CS_PIN              GPIO_PIN_0
#define TM4C_SD_INIT_HZ             400000UL
#define TM4C_SD_TRANSFER_HZ         12000000UL

#endif
