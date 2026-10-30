#include <stdbool.h>
#include <stdint.h>

#include "config.h"
#include "doomgeneric.h"
#include "gpio_input.h"
#include "lcd_init.h"

#include "i_video.h"

#include "driverlib/interrupt.h"
#include "driverlib/systick.h"

static volatile uint32_t s_ticks_ms;
static uint32_t s_system_clock_hz = TM4C_SYSTEM_CLOCK_HZ;
static uint16_t s_palette565[256];

static inline uint16_t RGB888ToRGB565(uint8_t r, uint8_t g, uint8_t b)
{
    return (uint16_t)(((uint16_t)(r & 0xF8U) << 8) |
                      ((uint16_t)(g & 0xFCU) << 3) |
                      ((uint16_t)b >> 3));
}

static void UpdatePaletteCache(void)
{
    if (!palette_changed)
    {
        return;
    }

    for (uint32_t i = 0; i < 256U; ++i)
    {
        s_palette565[i] = RGB888ToRGB565(colors[i].r, colors[i].g, colors[i].b);
    }

    palette_changed = false;
}

void TM4C_SetSystemClockHz(uint32_t system_clock_hz)
{
    if (system_clock_hz != 0U)
    {
        s_system_clock_hz = system_clock_hz;
    }
}

void SysTick_Handler(void)
{
    ++s_ticks_ms;
}

void DG_Init(void)
{
    LCD_Clear(0U);
    {
        volatile uint16_t *back = LCD_GetBackBuffer();

        for (uint32_t i = 0; i < (TM4C_LCD_WIDTH * TM4C_LCD_HEIGHT); ++i)
        {
            back[i] = 0U;
        }
    }
    s_ticks_ms = 0U;
    SysTickPeriodSet(s_system_clock_hz / 1000U);
    SysTickIntEnable();
    SysTickEnable();
    IntMasterEnable();
}

void DG_DrawFrame(void)
{
    volatile uint16_t *dst = LCD_GetBackBuffer();
    const uint8_t *src = (const uint8_t *)DG_ScreenBuffer;

    UpdatePaletteCache();

    /* Panel width == Doom width, so the Doom image is a contiguous block
       starting at the top-left corner (framebuffer address 0x60000000). */
    for (uint32_t i = 0; i < (TM4C_DOOM_WIDTH * TM4C_DOOM_HEIGHT); ++i)
    {
        dst[i] = s_palette565[src[i]];
    }

    LCD_SwapBuffers();
}

void DG_SleepMs(uint32_t ms)
{
    uint32_t deadline = s_ticks_ms + ms;

    while ((int32_t)(deadline - s_ticks_ms) > 0)
    {
    }
}

uint32_t DG_GetTicksMs(void)
{
    return s_ticks_ms;
}

int DG_GetKey(int *pressed, unsigned char *key)
{
    (void)pressed; (void)key; return 0;  // GPIO_InputPollEvent dočasně vypnuto
}

void DG_SetWindowTitle(const char *title)
{
    (void)title;
}
