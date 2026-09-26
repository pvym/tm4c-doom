#include "config.h"
#include "lcd_init.h"

volatile uint16_t *LCD_GetFramebuffer(void)
{
    return (volatile uint16_t *)(uintptr_t)TM4C_LCD_FRAMEBUFFER_ADDR;
}

void __attribute__((weak)) TM4C_LCD_ControllerInit(void)
{
}

void LCD_Clear(uint16_t color)
{
    volatile uint16_t *framebuffer = LCD_GetFramebuffer();

    for (uint32_t i = 0; i < (TM4C_LCD_WIDTH * TM4C_LCD_HEIGHT); ++i)
    {
        framebuffer[i] = color;
    }
}

void LCD_Init(void)
{
    TM4C_LCD_ControllerInit();
    LCD_Clear(0U);
}
