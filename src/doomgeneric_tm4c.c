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
#ifdef TM4C_LCD_TEST_PATTERN
    /* Mřížka 1 px čar po 16 px v landscape rastru 480x320 (stride 480). */
    for (uint32_t b = 0U; b < 2U; ++b)
    {
        volatile uint16_t *fb = (volatile uint16_t *)(uintptr_t)
            (b == 0U ? TM4C_LCD_FRAMEBUFFER_ADDR : TM4C_LCD_FRAMEBUFFER2_ADDR);

        for (uint32_t y = 0U; y < TM4C_LCD_WIDTH; ++y)
        {
            for (uint32_t x = 0U; x < TM4C_LCD_HEIGHT; ++x)
            {
                fb[y * TM4C_LCD_HEIGHT + x] =
                    (((x & 15U) == 0U) || ((y & 15U) == 0U) ||
                     (x == (TM4C_LCD_HEIGHT - 1U)) || (y == (TM4C_LCD_WIDTH - 1U))) ? 0xFFFFU : 0U;
            }
        }
    }
    for (;;)
    {
    }
#endif
    s_ticks_ms = 0U;
    SysTickPeriodSet(s_system_clock_hz / 1000U);
    SysTickIntEnable();
    SysTickEnable();
    IntMasterEnable();
}

volatile uint32_t g_doom_frames;
volatile uint32_t g_doom_fps;
static uint32_t s_fps_last_ms;
static uint32_t s_fps_last_frames;

void DG_DrawFrame(void)
{
    volatile uint16_t *dst = LCD_GetBackBuffer();
    extern byte *I_VideoBuffer;
    const uint8_t *src = (const uint8_t *)I_VideoBuffer;

    UpdatePaletteCache();

    /* Center Doom's 320x200 frame in the 480x320 landscape display. */
    {
        volatile uint32_t *dst32 = (volatile uint32_t *)dst;
        const uint16_t *pal = s_palette565;
        const uint32_t dst_stride = TM4C_LCD_HEIGHT;
        const uint32_t x_offset = (TM4C_LCD_HEIGHT - TM4C_DOOM_WIDTH) / 2U;
        const uint32_t y_offset = (TM4C_LCD_WIDTH - TM4C_DOOM_HEIGHT) / 2U;

        // Dva pixely na jeden 32bitový zápis; pauza omezuje podtečení LCD DMA.
        for (uint32_t y = 0U; y < TM4C_DOOM_HEIGHT; ++y)
        {
            const uint32_t dst_row = (y + y_offset) * dst_stride + x_offset;

            for (uint32_t x = 0U; x < TM4C_DOOM_WIDTH; x += 2U)
            {
                const uint32_t src_index = (y * TM4C_DOOM_WIDTH) + x;
                const uint32_t dst_index = dst_row + x;

                dst32[dst_index >> 1] =
                    (uint32_t)pal[src[src_index]] |
                    ((uint32_t)pal[src[src_index + 1U]] << 16);
            }

            for (volatile uint32_t d = 0U; d < TM4C_LCD_COPY_PAUSE; ++d)
            {
            }
        }
    }

    LCD_SwapBuffers();

    ++g_doom_frames;
    if ((s_ticks_ms - s_fps_last_ms) >= 1000U)
    {
        g_doom_fps = ((g_doom_frames - s_fps_last_frames) * 1000U) / (s_ticks_ms - s_fps_last_ms);
        s_fps_last_ms = s_ticks_ms;
        s_fps_last_frames = g_doom_frames;
    }
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
